#include "Runtime/DialogueRunner.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Runtime/DialogueConditionEvaluator.h"
#include "Runtime/DialogueTextParser.h"
#include "Runtime/VariableBank.h"

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
        const FDialogueNode* Node = CurrentTree->FindNode(CurrentNodeGuid);
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
        const FDialogueNode* Node = CurrentTree->FindNode(CurrentNodeGuid);
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

    const FDialogueNode* Node = CurrentTree->FindNode(CurrentNodeGuid);
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
    if (!CurrentTree || !CurrentTree->FindNode(NodeGuid))
    {
        return false;
    }

    CurrentNodeGuid = NodeGuid;
    CurrentLineIndex = INDEX_NONE;
    PushMemento();
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

    TArray<FDialogueEdge> Edges;
    CurrentTree->GetOutgoingEdges(CurrentNodeGuid, Edges);

    const FDialogueEdge* FallbackEdge = nullptr;
    for (const FDialogueEdge& Edge : Edges)
    {
        if (Edge.FromSlotIndex != SlotIndex)
        {
            continue;
        }

        if (Edge.ConditionExpression.TrimStartAndEnd().IsEmpty())
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

bool UDialogueRunner::EvaluateEdgeCondition(const FDialogueEdge& Edge) const
{
    bool bSuccess = false;
    return UDialogueConditionEvaluator::EvaluateCondition(Edge.ConditionExpression, VariableBank, bSuccess) && bSuccess;
}

bool UDialogueRunner::EvaluateChoiceCondition(const FDialogueChoice& Choice) const
{
    bool bSuccess = false;
    return UDialogueConditionEvaluator::EvaluateCondition(Choice.VisibilityCondition, VariableBank, bSuccess) && bSuccess;
}

bool UDialogueRunner::SelectConditionBranch(const FDialogueNode& Node, FDialogueEdge& OutEdge) const
{
    if (!CurrentTree)
    {
        return false;
    }

    TArray<FDialogueEdge> Edges;
    CurrentTree->GetOutgoingEdges(Node.NodeGuid, Edges);

    const FDialogueEdge* DefaultEdge = nullptr;
    for (const FDialogueEdge& Edge : Edges)
    {
        if (Edge.ConditionExpression.TrimStartAndEnd().IsEmpty())
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

    if (Edges.IsValidIndex(Node.DefaultOutputIndex))
    {
        OutEdge = Edges[Node.DefaultOutputIndex];
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

