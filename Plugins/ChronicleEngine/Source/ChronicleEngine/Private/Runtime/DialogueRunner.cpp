#include "Runtime/DialogueRunner.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Runtime/DialogueConditionEvaluator.h"
#include "Runtime/DialogueTextParser.h"
#include "Runtime/VariableBank.h"

namespace
{
bool IsBlankConditionExpression(const FString& Expression)
{
    for (int32 Index = 0; Index < Expression.Len(); ++Index)
    {
        if (!FChar::IsWhitespace(Expression[Index]))
        {
            return false;
        }
    }

    return true;
}
}

UDialogueRunner::UDialogueRunner()
{
}

void UDialogueRunner::Initialize(UDialogueDatabase* InDatabase)
{
    Database = InDatabase;

    if (!VariableBank)
    {
        VariableBank = NewObject<UVariableBank>(this, TEXT("VariableBank"));
    }

    const TArray<FVariableDefinition> EmptyLocalDefinitions;
    VariableBank->InitializeFromDefinitions(Database ? Database->GlobalVariables : EmptyLocalDefinitions, EmptyLocalDefinitions);
    SetRunnerState(EDialogueRunnerState::Idle);
}

void UDialogueRunner::StartDialogue(UDialogueTree* Tree, FName EntryNode)
{
    if (!Tree)
    {
        return;
    }

    if (!VariableBank)
    {
        Initialize(Database);
    }

    FGuid EntryGuid;
    if (!Tree->ResolveEntryNode(EntryNode, EntryGuid))
    {
        return;
    }

    CurrentTree = Tree;
    CurrentNodeGuid = EntryGuid;
    CurrentLineIndex = INDEX_NONE;
    PresentedChoiceSlots.Reset();
    History.Reset();
    MementoStack.Reset();
    WaitingEventTag = FGameplayTag();
    ConditionResultCache.Reset();
    BuildRuntimeLookup();
    History.Reserve(64);
    MementoStack.Reserve(50);
    SeenDialogueHashes.Reserve(64);
    PresentedChoiceSlots.Reserve(8);

    const TArray<FVariableDefinition> EmptyGlobalDefinitions;
    VariableBank->InitializeFromDefinitions(Database ? Database->GlobalVariables : EmptyGlobalDefinitions, Tree->Variables);

    SetRunnerState(EDialogueRunnerState::Running);
    PushMemento();
    OnDialogueStarted.Broadcast(CurrentTree);
    ProcessCurrentNode();
}

void UDialogueRunner::Advance()
{
    if (!CurrentTree)
    {
        return;
    }

    if (RunnerState == EDialogueRunnerState::WaitingForInput)
    {
        const FDialogueNode* Node = FindRuntimeNode(CurrentNodeGuid);
        if (Node && Node->NodeType == EDialogueNodeType::Speech && Node->Lines.IsValidIndex(CurrentLineIndex + 1))
        {
            ++CurrentLineIndex;
            PresentCurrentLine();
            return;
        }

        SetRunnerState(EDialogueRunnerState::Running);
        if (FollowFirstEdge())
        {
            ProcessCurrentNode();
        }
        else
        {
            EndDialogue();
        }
        return;
    }

    if (RunnerState == EDialogueRunnerState::Running)
    {
        ProcessCurrentNode();
    }
}

void UDialogueRunner::SelectChoice(int32 ChoiceIndex)
{
    if (RunnerState != EDialogueRunnerState::WaitingForChoice || !PresentedChoiceSlots.IsValidIndex(ChoiceIndex))
    {
        return;
    }

    const int32 SlotIndex = PresentedChoiceSlots[ChoiceIndex];
    SetRunnerState(EDialogueRunnerState::Running);
    if (FollowEdgeBySlot(SlotIndex))
    {
        ProcessCurrentNode();
    }
    else
    {
        EndDialogue();
    }
}

void UDialogueRunner::EndDialogue()
{
    UDialogueTree* FinishedTree = CurrentTree;
    CurrentTree = nullptr;
    CurrentNodeGuid.Invalidate();
    CurrentLineIndex = INDEX_NONE;
    WaitingEventTag = FGameplayTag();
    PresentedChoiceSlots.Reset();
    ConditionResultCache.Reset();
    ResetRuntimeLookup();
    SetRunnerState(EDialogueRunnerState::Idle);
    OnDialogueEnded.Broadcast(FinishedTree);
}

void UDialogueRunner::NotifyEventComplete(FGameplayTag EventTag)
{
    if (RunnerState != EDialogueRunnerState::WaitingForEvent || WaitingEventTag != EventTag)
    {
        return;
    }

    WaitingEventTag = FGameplayTag();
    SetRunnerState(EDialogueRunnerState::Running);
    if (FollowFirstEdge())
    {
        ProcessCurrentNode();
    }
    else
    {
        EndDialogue();
    }
}

void UDialogueRunner::SetVariable(FGameplayTag Tag, const FVariableValue& Value, EChronicleVariableScope Scope)
{
    if (!VariableBank)
    {
        VariableBank = NewObject<UVariableBank>(this, TEXT("VariableBank"));
    }

    ConditionResultCache.Reset();
    VariableBank->SetVariable(Tag, Value, Scope);
}

FVariableValue UDialogueRunner::GetVariable(FGameplayTag Tag, bool& bFound) const
{
    bFound = false;
    return VariableBank ? VariableBank->GetVariable(Tag, bFound) : FVariableValue();
}

void UDialogueRunner::SaveState(FDialogueSaveData& OutData) const
{
    OutData.CurrentTreeGuid = CurrentTree ? CurrentTree->TreeGuid : FGuid();
    OutData.CurrentNodeGuid = CurrentNodeGuid;
    OutData.CurrentLineIndex = CurrentLineIndex;
    OutData.RunnerState = RunnerState;
    OutData.GlobalVariables = VariableBank ? VariableBank->GetGlobalVariables() : TMap<FGameplayTag, FVariableValue>();
    OutData.LocalVariables = VariableBank ? VariableBank->GetLocalVariables() : TMap<FGameplayTag, FVariableValue>();
    OutData.History = History;
    OutData.SeenDialogueHashes = SeenDialogueHashes;
}

void UDialogueRunner::LoadState(const FDialogueSaveData& InData)
{
    if (!VariableBank)
    {
        VariableBank = NewObject<UVariableBank>(this, TEXT("VariableBank"));
    }

    VariableBank->RestoreSnapshot(InData.GlobalVariables, InData.LocalVariables);
    ConditionResultCache.Reset();
    CurrentNodeGuid = InData.CurrentNodeGuid;
    CurrentLineIndex = InData.CurrentLineIndex;
    History = InData.History;
    SeenDialogueHashes = InData.SeenDialogueHashes;
    SetRunnerState(InData.RunnerState);
}

void UDialogueRunner::PerformRollback(int32 Steps)
{
    if (MementoStack.Num() <= 1)
    {
        return;
    }

    const int32 ClampedSteps = FMath::Clamp(Steps, 1, MementoStack.Num() - 1);
    const int32 TargetIndex = MementoStack.Num() - 1 - ClampedSteps;
    const FDialogueSaveData TargetState = MementoStack[TargetIndex];
    MementoStack.SetNum(TargetIndex + 1);
    LoadState(TargetState);
}

void UDialogueRunner::SetRunnerState(EDialogueRunnerState NewState)
{
    if (RunnerState == NewState)
    {
        return;
    }

    const EDialogueRunnerState OldState = RunnerState;
    RunnerState = NewState;
    OnRunnerStateChanged.Broadcast(OldState, NewState);
}

void UDialogueRunner::ProcessCurrentNode()
{
    if (!CurrentTree)
    {
        return;
    }

    constexpr int32 MaxTraversalSteps = 1024;
    for (int32 Step = 0; Step < MaxTraversalSteps; ++Step)
    {
        const FDialogueNode* Node = FindRuntimeNode(CurrentNodeGuid);
        if (!Node)
        {
            EndDialogue();
            return;
        }

        switch (Node->NodeType)
        {
        case EDialogueNodeType::Root:
        case EDialogueNodeType::Sequence:
            if (!FollowFirstEdge())
            {
                EndDialogue();
                return;
            }
            break;

        case EDialogueNodeType::Speech:
            if (Node->Lines.Num() == 0)
            {
                if (!FollowFirstEdge())
                {
                    EndDialogue();
                    return;
                }
                break;
            }
            CurrentLineIndex = 0;
            PresentCurrentLine();
            return;

        case EDialogueNodeType::Choice:
        {
            PresentedChoiceSlots.Reset();
            TArray<FDialogueChoice> VisibleChoices;
            VisibleChoices.Reserve(Node->Choices.Num());
            PresentedChoiceSlots.Reserve(Node->Choices.Num());
            for (int32 Index = 0; Index < Node->Choices.Num(); ++Index)
            {
                if (EvaluateChoiceCondition(Node->Choices[Index]))
                {
                    VisibleChoices.Add(Node->Choices[Index]);
                    PresentedChoiceSlots.Add(Index);
                }
            }

            if (VisibleChoices.Num() == 0)
            {
                if (!FollowFirstEdge())
                {
                    EndDialogue();
                    return;
                }
                break;
            }

            if (Node->bAutoSelectIfSingle && VisibleChoices.Num() == 1)
            {
                if (!FollowEdgeBySlot(PresentedChoiceSlots[0]))
                {
                    EndDialogue();
                    return;
                }
                break;
            }

            OnChoicesPresented.Broadcast(VisibleChoices);
            SetRunnerState(EDialogueRunnerState::WaitingForChoice);
            PushMemento();
            return;
        }

        case EDialogueNodeType::Condition:
        {
            FDialogueEdge SelectedEdge;
            if (!SelectConditionBranch(*Node, SelectedEdge))
            {
                EndDialogue();
                return;
            }
            FollowEdge(SelectedEdge);
            break;
        }

        case EDialogueNodeType::Event:
        {
            FDialogueEventData EventData;
            EventData.EventTag = Node->EventTag;
            EventData.Payload = Node->EventPayload;
            EventData.bAsync = Node->bEventIsAsync;
            OnDialogueEvent.Broadcast(EventData);
            ConditionResultCache.Reset();

            if (Node->bEventIsAsync)
            {
                WaitingEventTag = Node->EventTag;
                SetRunnerState(EDialogueRunnerState::WaitingForEvent);
                PushMemento();
                return;
            }

            if (!FollowFirstEdge())
            {
                EndDialogue();
                return;
            }
            break;
        }

        case EDialogueNodeType::Wait:
            SetRunnerState(EDialogueRunnerState::WaitingForInput);
            PushMemento();
            return;

        default:
            if (!FollowFirstEdge())
            {
                EndDialogue();
                return;
            }
            break;
        }
    }

    EndDialogue();
}

void UDialogueRunner::PresentCurrentLine()
{
    if (!CurrentTree)
    {
        return;
    }

    const FDialogueNode* Node = FindRuntimeNode(CurrentNodeGuid);
    if (!Node || !Node->Lines.IsValidIndex(CurrentLineIndex))
    {
        EndDialogue();
        return;
    }

    FDialogueLine Line = Node->Lines[CurrentLineIndex];
    if (Line.Segments.Num() == 0)
    {
        UDialogueTextParser::ParseInlineTags(Line.Text, Line.Segments);
    }

    FDialogueHistoryEntry Entry;
    Entry.Timestamp = FDateTime::UtcNow();
    Entry.TreeGuid = CurrentTree->TreeGuid;
    Entry.NodeGuid = CurrentNodeGuid;
    Entry.LineID = Line.LineID;
    Entry.SpeakerTag = Line.SpeakerTag;
    Entry.Text = Line.Text;
    History.Add(Entry);
    SeenDialogueHashes.AddUnique(MakeSeenHash(Line));

    OnLineStarted.Broadcast(Line, ETextRevealMode::Typewriter);
    SetRunnerState(EDialogueRunnerState::WaitingForInput);
    PushMemento();
}

bool UDialogueRunner::MoveToNode(const FGuid& NodeGuid)
{
    if (!CurrentTree || !FindRuntimeNode(NodeGuid))
    {
        return false;
    }

    CurrentNodeGuid = NodeGuid;
    CurrentLineIndex = INDEX_NONE;
    return true;
}

bool UDialogueRunner::FollowFirstEdge()
{
    return FollowEdgeBySlot(0);
}

bool UDialogueRunner::FollowEdgeBySlot(int32 SlotIndex)
{
    if (!CurrentTree)
    {
        return false;
    }

    const TArray<int32>* EdgeIndices = FindRuntimeOutgoingEdges(CurrentNodeGuid);
    if (!EdgeIndices)
    {
        return false;
    }

    const FDialogueEdge* FallbackEdge = nullptr;
    for (const int32 EdgeIndex : *EdgeIndices)
    {
        const FDialogueEdge& Edge = CurrentTree->Edges[EdgeIndex];
        if (Edge.FromSlotIndex != SlotIndex)
        {
            continue;
        }

        if (IsBlankConditionExpression(Edge.ConditionExpression))
        {
            FallbackEdge = &Edge;
            continue;
        }

        if (EvaluateEdgeCondition(Edge))
        {
            return FollowEdge(Edge);
        }
    }

    return FallbackEdge ? FollowEdge(*FallbackEdge) : false;
}

bool UDialogueRunner::FollowEdge(const FDialogueEdge& Edge)
{
    return MoveToNode(Edge.ToNodeGuid);
}

bool UDialogueRunner::EvaluateConditionExpression(const FString& Expression) const
{
    if (IsBlankConditionExpression(Expression))
    {
        return true;
    }

    const FString* CacheKey = &Expression;
    FString TrimmedExpression;
    if (FChar::IsWhitespace(Expression[0]) || FChar::IsWhitespace(Expression[Expression.Len() - 1]))
    {
        TrimmedExpression = Expression.TrimStartAndEnd();
        CacheKey = &TrimmedExpression;
    }

    if (const bool* CachedResult = ConditionResultCache.Find(*CacheKey))
    {
        return *CachedResult;
    }

    bool bSuccess = false;
    const bool bResult = UDialogueConditionEvaluator::EvaluateCondition(*CacheKey, VariableBank, bSuccess) && bSuccess;
    if (bSuccess)
    {
        ConditionResultCache.Add(*CacheKey, bResult);
    }
    return bResult;
}

bool UDialogueRunner::EvaluateEdgeCondition(const FDialogueEdge& Edge) const
{
    return EvaluateConditionExpression(Edge.ConditionExpression);
}

bool UDialogueRunner::EvaluateChoiceCondition(const FDialogueChoice& Choice) const
{
    return EvaluateConditionExpression(Choice.VisibilityCondition);
}

bool UDialogueRunner::SelectConditionBranch(const FDialogueNode& Node, FDialogueEdge& OutEdge) const
{
    if (!CurrentTree)
    {
        return false;
    }

    const TArray<int32>* EdgeIndices = FindRuntimeOutgoingEdges(Node.NodeGuid);
    if (!EdgeIndices)
    {
        return false;
    }

    const FDialogueEdge* DefaultEdge = nullptr;
    for (const int32 EdgeIndex : *EdgeIndices)
    {
        const FDialogueEdge& Edge = CurrentTree->Edges[EdgeIndex];
        if (IsBlankConditionExpression(Edge.ConditionExpression))
        {
            DefaultEdge = &Edge;
            continue;
        }

        if (EvaluateEdgeCondition(Edge))
        {
            OutEdge = Edge;
            return true;
        }
    }

    if (DefaultEdge)
    {
        OutEdge = *DefaultEdge;
        return true;
    }

    if (EdgeIndices->IsValidIndex(Node.DefaultOutputIndex))
    {
        OutEdge = CurrentTree->Edges[(*EdgeIndices)[Node.DefaultOutputIndex]];
        return true;
    }

    return false;
}

void UDialogueRunner::PushMemento()
{
    FDialogueSaveData Snapshot;
    SaveState(Snapshot);
    MementoStack.Add(Snapshot);

    constexpr int32 MaxMementos = 50;
    if (MementoStack.Num() > MaxMementos)
    {
        MementoStack.RemoveAt(0);
    }
}

FString UDialogueRunner::MakeSeenHash(const FDialogueLine& Line) const
{
    return FString::Printf(TEXT("%s:%s:%s"),
        CurrentTree ? *CurrentTree->TreeGuid.ToString(EGuidFormats::DigitsWithHyphens) : TEXT("NoTree"),
        *CurrentNodeGuid.ToString(EGuidFormats::DigitsWithHyphens),
        *Line.LineID.ToString());
}

void UDialogueRunner::BuildRuntimeLookup()
{
    RuntimeNodeIndices.Reset();
    RuntimeOutgoingEdgeIndices.Reset();

    if (!CurrentTree)
    {
        return;
    }

    RuntimeNodeIndices.Reserve(CurrentTree->Nodes.Num());
    for (int32 NodeIndex = 0; NodeIndex < CurrentTree->Nodes.Num(); ++NodeIndex)
    {
        RuntimeNodeIndices.Add(CurrentTree->Nodes[NodeIndex].NodeGuid, NodeIndex);
    }

    RuntimeOutgoingEdgeIndices.Reserve(CurrentTree->Edges.Num());
    for (int32 EdgeIndex = 0; EdgeIndex < CurrentTree->Edges.Num(); ++EdgeIndex)
    {
        const FDialogueEdge& Edge = CurrentTree->Edges[EdgeIndex];
        RuntimeOutgoingEdgeIndices.FindOrAdd(Edge.FromNodeGuid).Add(EdgeIndex);
    }

    for (TPair<FGuid, TArray<int32>>& EdgePair : RuntimeOutgoingEdgeIndices)
    {
        EdgePair.Value.Sort([this](const int32 LeftIndex, const int32 RightIndex)
        {
            const FDialogueEdge& Left = CurrentTree->Edges[LeftIndex];
            const FDialogueEdge& Right = CurrentTree->Edges[RightIndex];
            if (Left.FromSlotIndex == Right.FromSlotIndex)
            {
                return Left.ToSlotIndex < Right.ToSlotIndex;
            }
            return Left.FromSlotIndex < Right.FromSlotIndex;
        });
    }
}

void UDialogueRunner::ResetRuntimeLookup()
{
    RuntimeNodeIndices.Reset();
    RuntimeOutgoingEdgeIndices.Reset();
}

const FDialogueNode* UDialogueRunner::FindRuntimeNode(const FGuid& NodeGuid) const
{
    if (!CurrentTree)
    {
        return nullptr;
    }

    if (const int32* NodeIndex = RuntimeNodeIndices.Find(NodeGuid))
    {
        return CurrentTree->Nodes.IsValidIndex(*NodeIndex) ? &CurrentTree->Nodes[*NodeIndex] : nullptr;
    }

    return CurrentTree->FindNode(NodeGuid);
}

const TArray<int32>* UDialogueRunner::FindRuntimeOutgoingEdges(const FGuid& NodeGuid) const
{
    return RuntimeOutgoingEdgeIndices.Find(NodeGuid);
}
