#include "Misc/AutomationTest.h"

#include "ChronicleTestListener.h"
#include "Data/DialogueTree.h"
#include "Data/DialogueTrigger.h"
#include "GameplayTagsManager.h"
#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Runtime/DialogueConditionEvaluator.h"
#include "Runtime/DialogueRunner.h"
#include "Runtime/DialogueTextParser.h"
#include "Runtime/DialogueTriggerManager.h"
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

void AddEdge(UDialogueTree* Tree, const FGuid& From, const FGuid& To, int32 Slot = 0, const FString& Condition = FString(), float Weight = 1.0f)
{
    FDialogueEdge Edge;
    Edge.FromNodeGuid = From;
    Edge.ToNodeGuid = To;
    Edge.FromSlotIndex = Slot;
    Edge.ConditionExpression = Condition;
    Edge.Weight = Weight;
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

UDialogueTree* MakeTwoLineTree()
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    const FGuid RootGuid = AddNode(Tree, EDialogueNodeType::Root);
    const FGuid SpeechGuid = AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueLine FirstLine;
    FirstLine.LineID = TEXT("Line_001");
    FirstLine.SpeakerTag = Tag(TEXT("Chronicle.Speaker.Alice"));
    FirstLine.Text = FText::FromString(TEXT("first"));
    FirstLine.VoiceID = TEXT("VO_First");
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(FirstLine);

    FDialogueLine SecondLine;
    SecondLine.LineID = TEXT("Line_002");
    SecondLine.SpeakerTag = Tag(TEXT("Chronicle.Speaker.Alice"));
    SecondLine.Text = FText::FromString(TEXT("second"));
    SecondLine.VoiceID = TEXT("VO_Second");
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(SecondLine);

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

UDialogueTrigger* MakeTrigger(UDialogueTree* Tree, const TCHAR* TriggerTagName, EDialogueTriggerType TriggerType, int32 Priority = 0)
{
    UDialogueTrigger* Trigger = NewObject<UDialogueTrigger>();
    Trigger->TriggerTag = Tag(TriggerTagName);
    Trigger->TargetTree = Tree;
    Trigger->TriggerType = TriggerType;
    Trigger->Priority = Priority;
    return Trigger;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerRandomWeightedTest, "Chronicle.Runtime.Runner.RandomWeighted", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerRandomWeightedTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid RandomGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Random);
    const FGuid ZeroWeightGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    const FGuid WeightedGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueLine ZeroWeightLine;
    ZeroWeightLine.LineID = TEXT("ZeroWeight");
    ZeroWeightLine.Text = FText::FromString(TEXT("zero"));
    Tree->FindNodeMutable(ZeroWeightGuid)->Lines.Add(ZeroWeightLine);

    FDialogueLine WeightedLine;
    WeightedLine.LineID = TEXT("Weighted");
    WeightedLine.Text = FText::FromString(TEXT("weighted"));
    Tree->FindNodeMutable(WeightedGuid)->Lines.Add(WeightedLine);

    ChronicleTests::AddEdge(Tree, RootGuid, RandomGuid);
    ChronicleTests::AddEdge(Tree, RandomGuid, ZeroWeightGuid, 0, FString(), 0.0f);
    ChronicleTests::AddEdge(Tree, RandomGuid, WeightedGuid, 1, FString(), 1.0f);

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("Random node honors positive edge weights"), Listener->LastLine.Text.ToString(), FString(TEXT("weighted")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerJumpTest, "Chronicle.Runtime.Runner.Jump", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerJumpTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid JumpGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Jump);
    const FGuid DecoyGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    const FGuid TargetGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueNode* JumpNode = Tree->FindNodeMutable(JumpGuid);
    JumpNode->TargetEntryNode = FName(TEXT("JumpTarget"));

    FDialogueLine DecoyLine;
    DecoyLine.LineID = TEXT("Decoy");
    DecoyLine.Text = FText::FromString(TEXT("decoy"));
    Tree->FindNodeMutable(DecoyGuid)->Lines.Add(DecoyLine);

    FDialogueLine TargetLine;
    TargetLine.LineID = TEXT("JumpTarget");
    TargetLine.Text = FText::FromString(TEXT("target"));
    Tree->FindNodeMutable(TargetGuid)->Lines.Add(TargetLine);

    ChronicleTests::AddEdge(Tree, RootGuid, JumpGuid);
    ChronicleTests::AddEdge(Tree, JumpGuid, DecoyGuid);

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("Jump node resolves a target entry instead of the default edge"), Listener->LastLine.Text.ToString(), FString(TEXT("target")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerSubDialogueReturnTest, "Chronicle.Runtime.Runner.SubDialogueReturn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerSubDialogueReturnTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTree* MainTree = NewObject<UDialogueTree>();
    MainTree->TreeGuid = FGuid::NewGuid();
    const FGuid MainRootGuid = ChronicleTests::AddNode(MainTree, EDialogueNodeType::Root);
    const FGuid SubDialogueGuid = ChronicleTests::AddNode(MainTree, EDialogueNodeType::SubDialogue);
    const FGuid AfterGuid = ChronicleTests::AddNode(MainTree, EDialogueNodeType::Speech);
    MainTree->RootNodeGuid = MainRootGuid;

    UDialogueTree* SubTree = NewObject<UDialogueTree>();
    SubTree->TreeGuid = FGuid::NewGuid();
    const FGuid SubRootGuid = ChronicleTests::AddNode(SubTree, EDialogueNodeType::Root);
    const FGuid SubLineGuid = ChronicleTests::AddNode(SubTree, EDialogueNodeType::Speech);
    SubTree->RootNodeGuid = SubRootGuid;

    FDialogueNode* SubDialogueNode = MainTree->FindNodeMutable(SubDialogueGuid);
    SubDialogueNode->TargetTree = SubTree;

    FDialogueLine SubLine;
    SubLine.LineID = TEXT("SubLine");
    SubLine.Text = FText::FromString(TEXT("inside sub"));
    SubTree->FindNodeMutable(SubLineGuid)->Lines.Add(SubLine);

    FDialogueLine AfterLine;
    AfterLine.LineID = TEXT("AfterSub");
    AfterLine.Text = FText::FromString(TEXT("after sub"));
    MainTree->FindNodeMutable(AfterGuid)->Lines.Add(AfterLine);

    ChronicleTests::AddEdge(MainTree, MainRootGuid, SubDialogueGuid);
    ChronicleTests::AddEdge(MainTree, SubDialogueGuid, AfterGuid);
    ChronicleTests::AddEdge(SubTree, SubRootGuid, SubLineGuid);

    Runner->StartDialogue(MainTree);
    TestEqual(TEXT("Sub-dialogue starts in the target tree"), Listener->LastLine.Text.ToString(), FString(TEXT("inside sub")));

    Runner->Advance();
    TestEqual(TEXT("Sub-dialogue returns to the calling tree"), Listener->LastLine.Text.ToString(), FString(TEXT("after sub")));
    TestTrue(TEXT("Runner is back on the main tree after return"), Runner->GetCurrentTree() == MainTree);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleRunnerCameraAnimationEventTest, "Chronicle.Runtime.Runner.CameraAnimationEvents", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleRunnerCameraAnimationEventTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid CameraGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Camera);
    const FGuid AnimationGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Animation);
    const FGuid SpeechGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    Tree->FindNodeMutable(CameraGuid)->EventPayload.Add(TEXT("Shot"), TEXT("CloseUp"));
    Tree->FindNodeMutable(AnimationGuid)->EventPayload.Add(TEXT("Montage"), TEXT("Nod"));

    FDialogueLine Line;
    Line.LineID = TEXT("AfterPresentationCue");
    Line.Text = FText::FromString(TEXT("after cue"));
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(Line);

    ChronicleTests::AddEdge(Tree, RootGuid, CameraGuid);
    ChronicleTests::AddEdge(Tree, CameraGuid, AnimationGuid);
    ChronicleTests::AddEdge(Tree, AnimationGuid, SpeechGuid);

    Runner->StartDialogue(Tree);

    TestEqual(TEXT("Camera and animation nodes broadcast presentation events"), Listener->EventCount, 2);
    if (Listener->EventHistory.Num() == 2)
    {
        TestEqual(TEXT("Camera node uses the default camera cue"), Listener->EventHistory[0].EventTag, ChronicleTests::Tag(TEXT("Chronicle.Camera.Cut")));
        TestEqual(TEXT("Animation node uses the default animation cue"), Listener->EventHistory[1].EventTag, ChronicleTests::Tag(TEXT("Chronicle.Animation.Play")));
        TestEqual(TEXT("Animation payload round-trips"), Listener->EventHistory[1].Payload.FindRef(TEXT("Montage")), FString(TEXT("Nod")));
    }
    TestEqual(TEXT("Presentation cue nodes continue to speech"), Listener->LastLine.Text.ToString(), FString(TEXT("after cue")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleTriggerManagerPriorityTest, "Chronicle.Runtime.TriggerManager.PriorityConditionsOneShot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleTriggerManagerPriorityTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);
    Runner->SetVariable(ChronicleTests::Tag(TEXT("Chronicle.Variable.Score")), FVariableValue::MakeInt(10));

    UDialogueTriggerManager* TriggerManager = NewObject<UDialogueTriggerManager>();
    TriggerManager->Initialize(Runner, nullptr);
    TriggerManager->OnTriggerActivated.AddDynamic(Listener, &UChronicleTestListener::HandleTriggerActivated);
    TriggerManager->OnTriggerRejected.AddDynamic(Listener, &UChronicleTestListener::HandleTriggerRejected);

    UDialogueTree* LowTree = ChronicleTests::MakeLinearTree(FText::FromString(TEXT("low priority")));
    UDialogueTree* HighTree = ChronicleTests::MakeLinearTree(FText::FromString(TEXT("high priority")));

    UDialogueTrigger* LowTrigger = ChronicleTests::MakeTrigger(LowTree, TEXT("Chronicle.Trigger.Low"), EDialogueTriggerType::Interact, 1);
    UDialogueTrigger* HighTrigger = ChronicleTests::MakeTrigger(HighTree, TEXT("Chronicle.Trigger.High"), EDialogueTriggerType::Interact, 10);
    HighTrigger->ActivationConditions.Add(TEXT("Chronicle.Variable.Score >= 5"));
    HighTrigger->bOneShot = true;

    TArray<UDialogueTrigger*> CandidateTriggers;
    CandidateTriggers.Add(LowTrigger);
    CandidateTriggers.Add(HighTrigger);

    TestTrue(TEXT("Best trigger activates"), TriggerManager->TryActivateBestTrigger(CandidateTriggers, EDialogueTriggerType::Interact));
    TestEqual(TEXT("Highest priority matching trigger wins"), Listener->LastLine.Text.ToString(), FString(TEXT("high priority")));
    TestEqual(TEXT("Trigger activation delegate fired"), Listener->TriggerActivatedCount, 1);
    TestEqual(TEXT("Activation payload carries trigger tag"), Listener->LastTriggerActivation.TriggerTag, ChronicleTests::Tag(TEXT("Chronicle.Trigger.High")));

    Runner->Advance();
    TestTrue(TEXT("One-shot trigger is consumed after dialogue ends"), TriggerManager->IsTriggerConsumed(ChronicleTests::Tag(TEXT("Chronicle.Trigger.High"))));
    TestFalse(TEXT("Consumed one-shot trigger cannot reactivate"), TriggerManager->TryActivateTrigger(HighTrigger));
    TestEqual(TEXT("Rejected delegate fired for consumed trigger"), Listener->TriggerRejectedCount, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleTriggerManagerCooldownTest, "Chronicle.Runtime.TriggerManager.TagCooldown", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleTriggerManagerCooldownTest::RunTest(const FString& Parameters)
{
    UChronicleTestListener* Listener = nullptr;
    UDialogueRunner* Runner = ChronicleTests::MakeRunner(Listener);

    UDialogueTriggerManager* TriggerManager = NewObject<UDialogueTriggerManager>();
    TriggerManager->Initialize(Runner, nullptr);
    TriggerManager->OnTriggerActivated.AddDynamic(Listener, &UChronicleTestListener::HandleTriggerActivated);
    TriggerManager->OnTriggerRejected.AddDynamic(Listener, &UChronicleTestListener::HandleTriggerRejected);

    UDialogueTree* Tree = ChronicleTests::MakeLinearTree(FText::FromString(TEXT("cooldown")));
    UDialogueTrigger* Trigger = ChronicleTests::MakeTrigger(Tree, TEXT("Chronicle.Trigger.Cooldown"), EDialogueTriggerType::Event, 0);
    Trigger->CooldownTime = 60.0f;
    TriggerManager->RegisterTrigger(Trigger);

    TestEqual(TEXT("Trigger manager registered one trigger"), TriggerManager->GetRegisteredTriggerCount(), 1);
    TestTrue(TEXT("Trigger activates by tag"), TriggerManager->TryActivateTriggerByTag(ChronicleTests::Tag(TEXT("Chronicle.Trigger.Cooldown"))));
    TestEqual(TEXT("Tag activation starts target dialogue"), Listener->LastLine.Text.ToString(), FString(TEXT("cooldown")));

    Runner->Advance();
    TestFalse(TEXT("Cooldown prevents immediate reactivation"), TriggerManager->TryActivateTriggerByTag(ChronicleTests::Tag(TEXT("Chronicle.Trigger.Cooldown"))));
    TestEqual(TEXT("Cooldown rejection delegate fired"), Listener->TriggerRejectedCount, 1);
    TestTrue(TEXT("Cooldown rejection reason is populated"), !Listener->LastTriggerRejectReason.IsEmpty());

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChroniclePresentationBacklogAutoRollbackTest, "Chronicle.Presentation.BacklogAutoRollback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChroniclePresentationBacklogAutoRollbackTest::RunTest(const FString& Parameters)
{
    UDialogueRunner* Runner = NewObject<UDialogueRunner>();
    Runner->Initialize(nullptr);

    UChronicleDialoguePresentationController* Controller = NewObject<UChronicleDialoguePresentationController>();
    Controller->BindRunner(Runner);

    UDialogueTree* Tree = ChronicleTests::MakeTwoLineTree();
    Controller->StartDialogue(Tree);

    TestEqual(TEXT("Presentation backlog captures first line"), Controller->GetBacklog().Num(), 1);
    TestEqual(TEXT("First line is exposed"), Controller->GetLastLine().Text.ToString(), FString(TEXT("first")));
    TestEqual(TEXT("Voice id is preserved for presentation"), Controller->GetLastLine().VoiceID, FName(TEXT("VO_First")));

    Controller->SetAutoAdvanceEnabled(true, 0.25f);
    Controller->TickPresentation(0.25f);

    TestEqual(TEXT("Auto advance presents second line"), Controller->GetLastLine().Text.ToString(), FString(TEXT("second")));
    TestEqual(TEXT("Backlog contains both lines"), Controller->GetBacklog().Num(), 2);

    Controller->RequestRollback(1);
    TestEqual(TEXT("Rollback syncs presentation backlog to runner history"), Controller->GetBacklog().Num(), 1);
    TestEqual(TEXT("Rollback leaves runner waiting on the restored line"), Runner->GetRunnerState(), EDialogueRunnerState::WaitingForInput);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChroniclePresentationSkipChoiceCueTest, "Chronicle.Presentation.SkipChoiceCue", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChroniclePresentationSkipChoiceCueTest::RunTest(const FString& Parameters)
{
    UDialogueRunner* Runner = NewObject<UDialogueRunner>();
    Runner->Initialize(nullptr);

    UChronicleDialoguePresentationController* Controller = NewObject<UChronicleDialoguePresentationController>();
    Controller->BindRunner(Runner);

    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();
    const FGuid RootGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Root);
    const FGuid CameraGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Event);
    const FGuid SpeechGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    const FGuid ChoiceGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Choice);
    const FGuid LongGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    const FGuid ShortGuid = ChronicleTests::AddNode(Tree, EDialogueNodeType::Speech);
    Tree->RootNodeGuid = RootGuid;

    FDialogueNode* CameraNode = Tree->FindNodeMutable(CameraGuid);
    CameraNode->EventTag = ChronicleTests::Tag(TEXT("Chronicle.Camera.Cut"));
    CameraNode->EventPayload.Add(TEXT("Shot"), TEXT("AutomationShot"));

    FDialogueLine FirstLine;
    FirstLine.LineID = TEXT("Skip_001");
    FirstLine.Text = FText::FromString(TEXT("skip first"));
    FirstLine.VoiceID = TEXT("VO_Skip_First");
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(FirstLine);

    FDialogueLine SecondLine;
    SecondLine.LineID = TEXT("Skip_002");
    SecondLine.Text = FText::FromString(TEXT("skip second"));
    SecondLine.VoiceID = TEXT("VO_Skip_Second");
    Tree->FindNodeMutable(SpeechGuid)->Lines.Add(SecondLine);

    FDialogueChoice LongChoice;
    LongChoice.Text = FText::FromString(TEXT("Long"));
    Tree->FindNodeMutable(ChoiceGuid)->Choices.Add(LongChoice);

    FDialogueChoice ShortChoice;
    ShortChoice.Text = FText::FromString(TEXT("Short"));
    Tree->FindNodeMutable(ChoiceGuid)->Choices.Add(ShortChoice);

    FDialogueLine LongLine;
    LongLine.LineID = TEXT("LongResult");
    LongLine.Text = FText::FromString(TEXT("long result"));
    Tree->FindNodeMutable(LongGuid)->Lines.Add(LongLine);

    FDialogueLine ShortLine;
    ShortLine.LineID = TEXT("ShortResult");
    ShortLine.Text = FText::FromString(TEXT("short result"));
    Tree->FindNodeMutable(ShortGuid)->Lines.Add(ShortLine);

    ChronicleTests::AddEdge(Tree, RootGuid, CameraGuid);
    ChronicleTests::AddEdge(Tree, CameraGuid, SpeechGuid);
    ChronicleTests::AddEdge(Tree, SpeechGuid, ChoiceGuid);
    ChronicleTests::AddEdge(Tree, ChoiceGuid, LongGuid, 0);
    ChronicleTests::AddEdge(Tree, ChoiceGuid, ShortGuid, 1);

    Controller->StartDialogue(Tree);
    TestEqual(TEXT("Camera cue is forwarded to presentation"), Controller->GetLastEventData().EventTag, ChronicleTests::Tag(TEXT("Chronicle.Camera.Cut")));
    TestEqual(TEXT("Camera payload is preserved"), Controller->GetLastEventData().Payload.FindRef(TEXT("Shot")), FString(TEXT("AutomationShot")));

    Controller->SetSkipModeEnabled(true);
    Controller->TickPresentation(0.0f);

    TestEqual(TEXT("Skip drains speech until choices"), Runner->GetRunnerState(), EDialogueRunnerState::WaitingForChoice);
    TestEqual(TEXT("Skip exposes instant reveal mode"), Controller->GetLastRevealMode(), ETextRevealMode::Instant);
    TestEqual(TEXT("Skip captures skipped lines in backlog"), Controller->GetBacklog().Num(), 2);
    TestEqual(TEXT("Choices are exposed through presentation controller"), Controller->GetPresentedChoices().Num(), 2);

    Controller->SelectChoice(1);
    TestEqual(TEXT("Choice selection forwards to selected branch"), Controller->GetLastLine().Text.ToString(), FString(TEXT("short result")));

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

    Runner->StartDialogue(Tree);
    Runner->EndDialogue();

    const double StartSeconds = FPlatformTime::Seconds();
    Runner->StartDialogue(Tree);
    const double ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;

    constexpr double EditorTraversalBudgetMs = 0.25;
    AddInfo(FString::Printf(TEXT("100-node condition traversal took %.4f ms in editor automation."), ElapsedMs));
    TestEqual(TEXT("Traversal reaches final line"), Listener->LastLine.Text.ToString(), FString(TEXT("done")));
    TestTrue(TEXT("100-node traversal stays within hardened editor smoke budget"), ElapsedMs < EditorTraversalBudgetMs);

    return true;
}
