#include "Misc/AutomationTest.h"

#include "ChronicleTestListener.h"
#include "Data/DialogueTree.h"
#include "GameplayTagsManager.h"
#include "Runtime/DialogueConditionEvaluator.h"
#include "Runtime/DialogueRunner.h"
#include "Runtime/DialogueTextParser.h"
#include "Runtime/VariableBank.h"
#include "HAL/PlatformTime.h"

namespace ChronicleTests
{
FGameplayTag Tag(const TCHAR* Name)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(FName(Name), false);
}

FGuid AddNode(UDialogueTree* Tree, EDialogueNodeType Type)
{
    FDialogueNode Node;
    Node.NodeGuid = FGuid::NewGuid();
    Node.NodeType = Type;
    Tree->Nodes.Add(Node);
    return Node.NodeGuid;
}

void AddEdge(UDialogueTree* Tree, const FGuid& From, const FGuid& To, int32 Slot = 0, const FString& Condition = FString())
{
    FDialogueEdge Edge;
    Edge.FromNodeGuid = From;
    Edge.ToNodeGuid = To;
    Edge.FromSlotIndex = Slot;
    Edge.ConditionExpression = Condition;
    Tree->Edges.Add(Edge);
}

UDialogueTree* MakeLinearTree(const FText& Text)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    const FGuid RootGuid = AddNode(Tree, EDialogueNodeType::Root);
    const FGuid SpeechGuid = AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueLine Line;
    Line.LineID = TEXT("Line_001");
    Line.SpeakerTag = Tag(TEXT("Chronicle.Speaker.Alice"));
    Line.Text = Text;
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(Line);

    AddEdge(Tree, RootGuid, SpeechGuid);
    return Tree;
}

UDialogueRunner* MakeRunner(UChronicleTestListener*& OutListener)
{
    UDialogueRunner* Runner = NewObject<UDialogueRunner>();
    Runner->Initialize(nullptr);

    OutListener = NewObject<UChronicleTestListener>();
    Runner->OnDialogueStarted.AddDynamic(OutListener, &UChronicleTestListener::HandleDialogueStarted);
    Runner->OnDialogueEnded.AddDynamic(OutListener, &UChronicleTestListener::HandleDialogueEnded);
    Runner->OnLineStarted.AddDynamic(OutListener, &UChronicleTestListener::HandleLineStarted);
    Runner->OnChoicesPresented.AddDynamic(OutListener, &UChronicleTestListener::HandleChoicesPresented);
    Runner->OnDialogueEvent.AddDynamic(OutListener, &UChronicleTestListener::HandleDialogueEvent);

    return Runner;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleVariableBankTest, "Chronicle.Runtime.VariableBank", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleVariableBankTest::RunTest(const FString& Parameters)
{
    UVariableBank* Bank = NewObject<UVariableBank>();
    const FGameplayTag ScoreTag = ChronicleTests::Tag(TEXT("Chronicle.Variable.Score"));
    TestTrue(TEXT("Score tag exists"), ScoreTag.IsValid());

    Bank->SetVariable(ScoreTag, FVariableValue::MakeInt(42));

    bool bFound = false;
    const FVariableValue Value = Bank->GetVariable(ScoreTag, bFound);
    TestTrue(TEXT("Variable was found"), bFound);
    TestEqual(TEXT("Variable value round-trips"), Value.IntValue, 42);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleConditionEvaluatorTest, "Chronicle.Runtime.ConditionEvaluator", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleConditionEvaluatorTest::RunTest(const FString& Parameters)
{
    UVariableBank* Bank = NewObject<UVariableBank>();
    Bank->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Score")), FVariableValue::MakeInt(75));
    Bank->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Flag")), FVariableValue::MakeBool(true));
    Bank->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Name")), FVariableValue::MakeString(TEXT("Alice")));

    bool bSuccess = false;
    const bool bResult = UDialogueConditionEvaluator::EvaluateCondition(
        TEXT("(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true) AND Chronicle.Variable.Name == \"Alice\""),
        Bank,
        bSuccess);

    TestTrue(TEXT("Expression parses"), bSuccess);
    TestTrue(TEXT("Expression evaluates true"), bResult);

    const bool bFalseResult = UDialogueConditionEvaluator::EvaluateCondition(TEXT("Chronicle.Variable.Score < 50 OR false"), Bank, bSuccess);
    TestTrue(TEXT("Second expression parses"), bSuccess);
    TestFalse(TEXT("Second expression evaluates false"), bFalseResult);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleInlineTagParserTest, "Chronicle.Runtime.InlineTagParser", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleInlineTagParserTest::RunTest(const FString& Parameters)
{
    TArray<FLineSegment> Segments;
    UDialogueTextParser::ParseInlineTags(FText::FromString(TEXT("Hello [color=#FF0000]red[wait=1.0] done")), Segments);

    TestTrue(TEXT("Parser emits multiple segments"), Segments.Num() >= 3);
    TestEqual(TEXT("First segment has plain text"), Segments[0].Text.ToString(), FString(TEXT("Hello ")));
    TestTrue(TEXT("Colored segment carries color tag"), Segments[1].Tags.Contains(TEXT("color")));
    TestTrue(TEXT("Wait segment is present"), Segments[2].Tags.Contains(TEXT("wait")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerLinearSpeechTest, "Chronicle.Runtime.Runner.LinearSpeech", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerLinearSpeechTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    UDialogueTree* Tree = ChronicleTests::MakeLinearTree(FText::FromString(TEXT("Hello, traveler.")));

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("Dialogue started"), Listener->StartedCount, 1);
    TestEqual(TEXT("Line started"), Listener->LineStartedCount, 1);
    TestEqual(TEXT("Runner waits for input"), Runner->GetRunnerState(), EDialogueRunnerState::WaitingForInput);
    TestEqual(TEXT("Line text matches"), Listener->LastLine.Text.ToString(), FString(TEXT("Hello, traveler.")));

    Runner->Advance();
    TestEqual(TEXT("Dialogue ended after final advance"), Runner->GetRunnerState(), EDialogueRunnerState::Idle);
    TestEqual(TEXT("End event fired"), Listener->EndedCount, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerConditionTest, "Chronicle.Runtime.Runner.ConditionBranch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerConditionTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    Runner->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Score")), FVariableValue::MakeInt(10));

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid ConditionGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Condition);
    const FGuid PassGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    const FGuid FailGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueLine PassLine;
    PassLine.LineID = TEXT("Pass");
    PassLine.Text = FText::FromString(TEXT("pass"));
    Tree->FindNodeMutable(PassGuid)->Lines.Add(PassLine);

    FDialogueLine FailLine;
    FailLine.LineID = TEXT("Fail");
    FailLine.Text = FText::FromString(TEXT("fail"));
    Tree->FindNodeMutable(FailGuid)->Lines.Add(FailLine);

    ChronicleTests::AddEdge(Tree, RootGuid, ConditionGuid);
    ChronicleTests::AddEdge(Tree, ConditionGuid, PassGuid, 0, TEXT("Chronicle.Variable.Score > 5"));
    ChronicleTests::AddEdge(Tree, ConditionGuid, FailGuid, 1);

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("Condition chooses pass branch"), Listener->LastLine.Text.ToString(), FString(TEXT("pass")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerChoiceTest, "Chronicle.Runtime.Runner.Choice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerChoiceTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    Runner->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Flag")), FVariableValue::MakeBool(true));

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid ChoiceGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Choice);
    const FGuid ResultGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueChoice HiddenChoice;
    HiddenChoice.Text = FText::FromString(TEXT("Hidden"));
    HiddenChoice.VisibilityCondition = TEXT("Chronicle.Variable.Flag == false");
    Tree->FindNodeMutable(ChoiceGuid)->Choices.Add(HiddenChoice);

    FDialogueChoice VisibleChoice;
    VisibleChoice.Text = FText::FromString(TEXT("Visible"));
    VisibleChoice.VisibilityCondition = TEXT("Chronicle.Variable.Flag == true");
    Tree->FindNodeMutable(ChoiceGuid)->Choices.Add(VisibleChoice);

    FDialogueLine ResultLine;
    ResultLine.LineID = TEXT("ChoiceResult");
    ResultLine.Text = FText::FromString(TEXT("chosen"));
    Tree->FindNodeMutable(ResultGuid)->Lines.Add(ResultLine);

    ChronicleTests::AddEdge(Tree, RootGuid, ChoiceGuid);
    ChronicleTests::AddEdge(Tree, ChoiceGuid, ResultGuid, 1);

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("One visible choice"), Listener->LastChoices.Num(), 1);
    if (Listener->LastChoices.IsValidIndex(0))
    {
        TestEqual(TEXT("Visible choice text"), Listener->LastChoices[0].Text.ToString(), FString(TEXT("Visible")));
    }

    Runner->SelectChoice(0);
    TestEqual(TEXT("Choice advances to speech"), Listener->LastLine.Text.ToString(), FString(TEXT("chosen")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerEventTest, "Chronicle.Runtime.Runner.Event", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerEventTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid EventGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Event);
    const FGuid SpeechGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueNode* EventNode = Tree->FindNodeMutable(EventGuid);
    EventNode->EventTag = ChronicleTests::Tag(TEXT("Chronicle.Event.Async"));
    EventNode->bEventIsAsync = true;
    EventNode->EventPayload.Add(TEXT("Key"), TEXT("Value"));

    FDialogueLine Line;
    Line.LineID = TEXT("AfterEvent");
    Line.Text = FText::FromString(TEXT("after"));
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(Line);

    ChronicleTests::AddEdge(Tree, RootGuid, EventGuid);
    ChronicleTests::AddEdge(Tree, EventGuid, SpeechGuid);

    Runner->StartDialogue(Tree);
    TestEqual(TEXT("Async event broadcasts"), Listener->EventCount, 1);
    TestEqual(TEXT("Runner waits for event"), Runner->GetRunnerState(), EDialogueRunnerState::WaitingForEvent);
    TestEqual(TEXT("Payload round-trips"), Listener->LastEvent.Payload.FindRef(TEXT("Key")), FString(TEXT("Value")));

    Runner->NotifyEventComplete(ChronicleTests::Tag(TEXT("Chronicle.Event.Async")));
    TestEqual(TEXT("Event completion advances"), Listener->LastLine.Text.ToString(), FString(TEXT("after")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerSaveRollbackTest, "Chronicle.Runtime.Runner.SaveRollback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerSaveRollbackTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    UDialogueTree* Tree = ChronicleTests::MakeLinearTree(FText::FromString(TEXT("snapshot")));

    const FGameplayTag ScoreTag = ChronicleTests::Tag(TEXT("Chronicle.Variable.Score"));
    Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(1));
    Runner->StartDialogue(Tree);

    FDialogueSaveData Saved;
    Runner->SaveState(Saved);
    Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(99));
    Runner->LoadState(Saved);

    bool bFound = false;
    const FVariableValue RestoredValue = Runner->GetVariable(ScoreTag, bFound);
    TestTrue(TEXT("Restored variable found"), bFound);
    TestEqual(TEXT("Restored variable value"), RestoredValue.IntValue, 1);

    Runner->PerformRollback(1);
    TestTrue(TEXT("Rollback leaves runner in a valid state"), Runner->GetRunnerState() == EDialogueRunnerState::Running || Runner->GetRunnerState() == EDialogueRunnerState::WaitingForInput);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerPerformanceTest, "Chronicle.Runtime.Performance.ConditionTraversal100", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerPerformanceTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    Runner->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Score")), FVariableValue::MakeInt(1));

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    Tree->RootNodeGuid = RootGuid;

    FGuid PreviousGuid = RootGuid;
    for (int32 Index = 0; Index < 100; ++Index)
    {
        const FGuid ConditionGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Condition);
        ChronicleTests::AddEdge(Tree, PreviousGuid, ConditionGuid);

        const FGuid NextGuid = (Index == 99) ? ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech) : ChronicleTests::AddNode(Tree, EDialogueNodeType::Condition);
        ChronicleTests::AddEdge(Tree, ConditionGuid, NextGuid, 0, TEXT("Chronicle.Variable.Score >= 0"));
        PreviousGuid = NextGuid;
    }

    FDialogueLine FinalLine;
    FinalLine.LineID = TEXT("PerfEnd");
    FinalLine.Text = FText::FromString(TEXT("done"));
    Tree->FindNodeMutable(PreviousGuid)->Lines.Add(FinalLine);

    const double StartSeconds = FPlatformTime::Seconds();
    Runner->StartDialogue(Tree);
    const double ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;

    AddInfo(FString::Printf(TEXT("100-node condition traversal took %.4f ms in editor automation."), ElapsedMs));
    TestEqual(TEXT("Traversal reaches final line"), Listener->LastLine.Text.ToString(), FString(TEXT("done")));
    TestTrue(TEXT("100-node traversal stays within editor smoke budget"), ElapsedMs < 5.0);

    return true;
}
