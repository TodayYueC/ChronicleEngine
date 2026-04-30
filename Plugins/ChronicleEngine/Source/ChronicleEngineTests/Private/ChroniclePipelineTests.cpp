#include "Misc/AutomationTest.h"

#include "Asset/ChronicleDialogueAuditLibrary.h"
#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Asset/DialogueImporterBase.h"
#include "ChronicleTestListener.h"
#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"
#include "Runtime/DialogueRunner.h"

namespace ChroniclePipelineTests
{
FGameplayTag Tag(const TCHAR* Name)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(FName(Name), false);
}

UDialogueTree* MakeTwoLineTree()
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FDialogueNode RootNode;
    RootNode.NodeGuid = FGuid::NewGuid();
    RootNode.NodeType = EDialogueNodeType::Root;
    Tree->RootNodeGuid = RootNode.NodeGuid;

    FDialogueNode FirstSpeech;
    FirstSpeech.NodeGuid = FGuid::NewGuid();
    FirstSpeech.NodeType = EDialogueNodeType::Speech;
    FDialogueLine FirstLine;
    FirstLine.LineID = TEXT("Line_001");
    FirstLine.SpeakerTag = Tag(TEXT("Chronicle.Speaker.Alice"));
    FirstLine.Text = FText::FromString(TEXT("Hello, traveler."));
    FirstLine.WaitTime = -1.0f;
    FirstSpeech.Lines.Add(FirstLine);

    FDialogueNode SecondSpeech;
    SecondSpeech.NodeGuid = FGuid::NewGuid();
    SecondSpeech.NodeType = EDialogueNodeType::Speech;
    FDialogueLine SecondLine;
    SecondLine.LineID = TEXT("Line_002");
    SecondLine.SpeakerTag = Tag(TEXT("Chronicle.Speaker.Alice"));
    SecondLine.Text = FText::FromString(TEXT("The tower wakes at dusk."));
    SecondLine.WaitTime = -1.0f;
    SecondSpeech.Lines.Add(SecondLine);

    Tree->Nodes.Add(RootNode);
    Tree->Nodes.Add(FirstSpeech);
    Tree->Nodes.Add(SecondSpeech);

    FDialogueEdge RootToFirst;
    RootToFirst.FromNodeGuid = RootNode.NodeGuid;
    RootToFirst.ToNodeGuid = FirstSpeech.NodeGuid;
    Tree->Edges.Add(RootToFirst);

    FDialogueEdge FirstToSecond;
    FirstToSecond.FromNodeGuid = FirstSpeech.NodeGuid;
    FirstToSecond.ToNodeGuid = SecondSpeech.NodeGuid;
    Tree->Edges.Add(FirstToSecond);

    return Tree;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleJsonRoundTripTest, "Chronicle.Pipeline.Json.RoundTrip", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleJsonRoundTripTest::RunTest(const FString& Parameters)
{
    UDialogueTree* SourceTree = ChroniclePipelineTests::MakeTwoLineTree();

    FString FirstJson;
    FString Error;
    TestTrue(TEXT("JSON export succeeds"), UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonString(SourceTree, FirstJson, Error));
    TestTrue(TEXT("JSON includes schema marker"), FirstJson.Contains(TEXT("ChronicleDialogueTree")));

    FString SecondJson;
    TestTrue(TEXT("Second JSON export succeeds"), UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonString(SourceTree, SecondJson, Error));
    TestEqual(TEXT("Repeated JSON export is stable"), SecondJson, FirstJson);

    UDialogueTree* ImportedTree = NewObject<UDialogueTree>();
    TestTrue(TEXT("JSON import succeeds"), UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonString(ImportedTree, FirstJson, Error));
    TestEqual(TEXT("Imported node count matches"), ImportedTree->Nodes.Num(), SourceTree->Nodes.Num());
    TestEqual(TEXT("Imported edge count matches"), ImportedTree->Edges.Num(), SourceTree->Edges.Num());

    const FDialogueNode* ImportedSpeech = ImportedTree->Nodes.FindByPredicate([](const FDialogueNode& Node)
    {
        return Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() > 0 && Node.Lines[0].LineID == TEXT("Line_001");
    });
    TestNotNull(TEXT("Imported first speech node found"), ImportedSpeech);
    if (ImportedSpeech)
    {
        TestEqual(TEXT("Imported text matches"), ImportedSpeech->Lines[0].Text.ToString(), FString(TEXT("Hello, traveler.")));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleCsvExportImportTest, "Chronicle.Pipeline.Csv.ExportImport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleCsvExportImportTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = ChroniclePipelineTests::MakeTwoLineTree();

    FString Csv;
    FString Error;
    TestTrue(TEXT("CSV export succeeds"), UChronicleDialogueJsonLibrary::ExportDialogueLinesToCsvString(Tree, Csv, Error));
    TestTrue(TEXT("CSV contains source text"), Csv.Contains(TEXT("Hello, traveler.")));
    TestTrue(TEXT("CSV header contains TranslatedText"), Csv.Contains(TEXT("TranslatedText")));

    const FDialogueNode* FirstSpeech = Tree->Nodes.FindByPredicate([](const FDialogueNode& Node)
    {
        return Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() > 0 && Node.Lines[0].LineID == TEXT("Line_001");
    });
    TestNotNull(TEXT("First speech node exists"), FirstSpeech);
    if (!FirstSpeech)
    {
        return false;
    }

    const FString ReplacementCsv = FString::Printf(
        TEXT("NodeGuid,LineIndex,LineID,SpeakerTag,SourceText,TranslatedText,EmotionTag,VoiceID,WaitTime,ContextComment\n")
        TEXT("%s,0,Line_001,Chronicle.Speaker.Alice,\"Hello, traveler.\",\"Greetings, traveler.\",,, -1.0,ManualUpdate\n"),
        *FirstSpeech->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));

    TestTrue(TEXT("CSV import succeeds"), UChronicleDialogueJsonLibrary::ImportDialogueLinesFromCsvString(Tree, ReplacementCsv, Error));
    const FDialogueNode* UpdatedNode = Tree->FindNode(FirstSpeech->NodeGuid);
    TestNotNull(TEXT("Updated node exists"), UpdatedNode);
    if (UpdatedNode)
    {
        TestEqual(TEXT("Translated text is imported"), UpdatedNode->Lines[0].Text.ToString(), FString(TEXT("Greetings, traveler.")));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleCsvScriptImportTest, "Chronicle.Pipeline.Csv.ScriptImport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleCsvScriptImportTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    const FString Csv =
        TEXT("LineID,SpeakerTag,Text,EmotionTag,VoiceID,NextLineID,ConditionExpression,EventTag,EventPayload,bEventIsAsync\n")
        TEXT("Line_001,Chronicle.Speaker.Alice,\"Hello from sheet\",,VO_001,Line_002,Chronicle.Variable.Flag == true,Chronicle.Event.Quest.Update,\"QuestTag=Chronicle.Quest.Main;ObjectiveIndex=1\",false\n")
        TEXT("Line_002,Chronicle.Speaker.Alice,\"Imported ending\",,VO_002,,,,,\n");

    UChronicleCsvDialogueImporter* Importer = NewObject<UChronicleCsvDialogueImporter>();
    FString Error;
    TestTrue(TEXT("CSV script importer succeeds"), Importer->ImportFromString(Tree, Csv, Error));
    TestEqual(TEXT("Script import creates root, speech nodes, and event node"), Tree->Nodes.Num(), 4);
    TestEqual(TEXT("Script import creates root, conditional event, and continuation edges"), Tree->Edges.Num(), 3);
    TestTrue(TEXT("Imported root node is valid"), Tree->RootNodeGuid.IsValid());

    const FDialogueNode* FirstSpeech = Tree->Nodes.FindByPredicate([](const FDialogueNode& Node)
    {
        return Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() > 0 && Node.Lines[0].LineID == TEXT("Line_001");
    });
    TestNotNull(TEXT("First imported speech exists"), FirstSpeech);
    if (FirstSpeech)
    {
        TestEqual(TEXT("Imported line text matches CSV"), FirstSpeech->Lines[0].Text.ToString(), FString(TEXT("Hello from sheet")));
        TestEqual(TEXT("Imported voice id matches CSV"), FirstSpeech->Lines[0].VoiceID, FName(TEXT("VO_001")));
    }

    const FDialogueNode* EventNode = Tree->Nodes.FindByPredicate([](const FDialogueNode& Node)
    {
        return Node.NodeType == EDialogueNodeType::Event;
    });
    TestNotNull(TEXT("Script import creates event node"), EventNode);
    if (EventNode)
    {
        TestEqual(TEXT("Event tag is imported"), EventNode->EventTag, ChroniclePipelineTests::Tag(TEXT("Chronicle.Event.Quest.Update")));
        TestEqual(TEXT("Event payload is imported"), EventNode->EventPayload.FindRef(TEXT("QuestTag")), FString(TEXT("Chronicle.Quest.Main")));
        TestEqual(TEXT("Event payload keeps additional fields"), EventNode->EventPayload.FindRef(TEXT("ObjectiveIndex")), FString(TEXT("1")));
    }

    UDialogueRunner* Runner = NewObject<UDialogueRunner>();
    Runner->Initialize(nullptr);
    Runner->SetVariable(ChroniclePipelineTests::Tag(TEXT("Chronicle.Variable.Flag")), FVariableValue::MakeBool(true));

    UChronicleTestListener* Listener = NewObject<UChronicleTestListener>();
    Runner->OnLineStarted.AddDynamic(Listener, &UChronicleTestListener::HandleLineStarted);
    Runner->OnDialogueEvent.AddDynamic(Listener, &UChronicleTestListener::HandleDialogueEvent);

    Runner->StartDialogue(Tree);
    TestEqual(TEXT("Imported script starts at first line"), Listener->LastLine.Text.ToString(), FString(TEXT("Hello from sheet")));

    Runner->Advance();
    TestEqual(TEXT("Imported condition allows event to fire"), Listener->EventCount, 1);
    TestEqual(TEXT("Imported script continues to target line"), Listener->LastLine.Text.ToString(), FString(TEXT("Imported ending")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleLocalizationGatherImportTest, "Chronicle.Pipeline.Localization.GatherImport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleLocalizationGatherImportTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = ChroniclePipelineTests::MakeTwoLineTree();

    FDialogueNode* FirstSpeech = Tree->Nodes.FindByPredicate([](const FDialogueNode& Node)
    {
        return Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() > 0 && Node.Lines[0].Text.ToString() == TEXT("Hello, traveler.");
    });
    TestNotNull(TEXT("First speech node exists"), FirstSpeech);
    if (!FirstSpeech)
    {
        return false;
    }

    FirstSpeech->Lines[0].LineID = NAME_None;

    int32 UpdatedLineIds = 0;
    FString Error;
    TestTrue(TEXT("Stable LineID generation succeeds"), UChronicleDialogueJsonLibrary::EnsureStableLineIds(Tree, UpdatedLineIds, Error));
    TestEqual(TEXT("One missing LineID was generated"), UpdatedLineIds, 1);
    TestFalse(TEXT("Generated LineID is valid"), FirstSpeech->Lines[0].LineID.IsNone());

    TArray<FChronicleDialogueLocalizationEntry> Entries;
    TestTrue(TEXT("Tree localization gather succeeds"), UChronicleDialogueJsonLibrary::GatherDialogueTextsFromTree(Tree, TEXT("Game.Dialogue"), TEXT("ja-JP"), Entries, Error));
    TestEqual(TEXT("Gather emits one entry per dialogue line"), Entries.Num(), 2);
    TestEqual(TEXT("Gather preserves namespace"), Entries[0].Namespace, FString(TEXT("Game.Dialogue")));
    TestEqual(TEXT("Gather preserves culture"), Entries[0].Culture, FString(TEXT("ja-JP")));
    TestTrue(TEXT("Gather uses line id as stable key"), Entries[0].Key == FirstSpeech->Lines[0].LineID || Entries[1].Key == FirstSpeech->Lines[0].LineID);

    FString LocalizationCsv;
    TestTrue(TEXT("Localization CSV export succeeds"), UChronicleDialogueJsonLibrary::ExportLocalizationCsvFromTree(Tree, TEXT("Game.Dialogue"), TEXT("ja-JP"), LocalizationCsv, Error));
    TestTrue(TEXT("Localization CSV includes Key header"), LocalizationCsv.Contains(TEXT("Key")));
    TestTrue(TEXT("Localization CSV includes context comments"), LocalizationCsv.Contains(TEXT("ContextComment")));

    const FString TranslationCsv = FString::Printf(
        TEXT("Namespace,Key,Culture,TreeGuid,NodeGuid,LineIndex,LineID,SpeakerTag,SourceText,TranslatedText,ContextComment\n")
        TEXT("Game.Dialogue,%s,ja-JP,%s,%s,0,%s,Chronicle.Speaker.Alice,\"Hello, traveler.\",\"こんにちは、旅人。\",ManualTranslation\n"),
        *FirstSpeech->Lines[0].LineID.ToString(),
        *Tree->TreeGuid.ToString(EGuidFormats::DigitsWithHyphens),
        *FirstSpeech->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens),
        *FirstSpeech->Lines[0].LineID.ToString());

    TestTrue(TEXT("Localization CSV import succeeds"), UChronicleDialogueJsonLibrary::ImportLocalizationCsvToTree(Tree, TranslationCsv, Error));
    TestEqual(TEXT("Translated text imported by stable key"), FirstSpeech->Lines[0].Text.ToString(), FString(TEXT("こんにちは、旅人。")));

    UDialogueDatabase* Database = NewObject<UDialogueDatabase>();
    Database->LocalizationSettings.Namespace = TEXT("Game.Dialogue");
    Database->DialogueTrees.Add(Tree);

    TArray<FChronicleDialogueLocalizationEntry> DatabaseEntries;
    TestTrue(TEXT("Database localization gather succeeds"), UChronicleDialogueJsonLibrary::GatherDialogueTextsFromDatabase(Database, TEXT("ja-JP"), DatabaseEntries, Error));
    TestEqual(TEXT("Database gather emits tree lines"), DatabaseEntries.Num(), 2);

    UDataTable* DefaultVoiceTable = NewObject<UDataTable>();
    UDataTable* JapaneseVoiceTable = NewObject<UDataTable>();
    Database->VoiceTable = DefaultVoiceTable;
    Database->CultureVoiceTables.Add(TEXT("ja-JP"), JapaneseVoiceTable);

    TestTrue(TEXT("Culture-specific voice table resolves"), Database->ResolveVoiceTableForCulture(TEXT("ja-JP")) == JapaneseVoiceTable);
    TestTrue(TEXT("Missing culture falls back to default voice table"), Database->ResolveVoiceTableForCulture(TEXT("en-US")) == DefaultVoiceTable);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleTreeValidationTest, "Chronicle.Pipeline.Validation.BrokenEdge", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleTreeValidationTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = ChroniclePipelineTests::MakeTwoLineTree();

    TArray<FChronicleDialogueValidationIssue> Issues;
    TestTrue(TEXT("Healthy tree validates"), UChronicleDialogueJsonLibrary::ValidateDialogueTree(Tree, Issues));
    TestEqual(TEXT("Healthy tree has no issues"), Issues.Num(), 0);

    FDialogueEdge BrokenEdge;
    BrokenEdge.FromNodeGuid = Tree->RootNodeGuid;
    BrokenEdge.ToNodeGuid = FGuid::NewGuid();
    Tree->Edges.Add(BrokenEdge);

    TestFalse(TEXT("Broken edge fails validation"), UChronicleDialogueJsonLibrary::ValidateDialogueTree(Tree, Issues));
    TestTrue(TEXT("Validation reports at least one issue"), Issues.Num() > 0);
    TestTrue(TEXT("Validation reports an error"), Issues.ContainsByPredicate([](const FChronicleDialogueValidationIssue& Issue)
    {
        return Issue.Severity == EChronicleDialogueValidationSeverity::Error;
    }));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleDialogueAuditReportTest, "Chronicle.Pipeline.Audit.Report", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleDialogueAuditReportTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FDialogueNode RootNode;
    RootNode.NodeGuid = FGuid::NewGuid();
    RootNode.NodeType = EDialogueNodeType::Root;
    Tree->RootNodeGuid = RootNode.NodeGuid;

    FDialogueNode SpeechNode;
    SpeechNode.NodeGuid = FGuid::NewGuid();
    SpeechNode.NodeType = EDialogueNodeType::Speech;
    FDialogueLine FirstLine;
    FirstLine.LineID = TEXT("Audit_001");
    FirstLine.SpeakerTag = ChroniclePipelineTests::Tag(TEXT("Chronicle.Speaker.Alice"));
    FirstLine.Text = FText::FromString(TEXT("Hello audit world"));
    SpeechNode.Lines.Add(FirstLine);
    FDialogueLine SecondLine;
    SecondLine.LineID = TEXT("Audit_002");
    SecondLine.SpeakerTag = ChroniclePipelineTests::Tag(TEXT("Chronicle.Speaker.Alice"));
    SecondLine.Text = FText::FromString(TEXT("Done now"));
    SpeechNode.Lines.Add(SecondLine);

    FDialogueNode ChoiceNode;
    ChoiceNode.NodeGuid = FGuid::NewGuid();
    ChoiceNode.NodeType = EDialogueNodeType::Choice;
    FDialogueChoice VisibleChoice;
    VisibleChoice.Text = FText::FromString(TEXT("Continue"));
    VisibleChoice.VisibilityCondition = TEXT("Chronicle.Variable.Score >= 10");
    ChoiceNode.Choices.Add(VisibleChoice);

    FDialogueNode EventNode;
    EventNode.NodeGuid = FGuid::NewGuid();
    EventNode.NodeType = EDialogueNodeType::Event;
    EventNode.EventTag = ChroniclePipelineTests::Tag(TEXT("Chronicle.Event.Quest.Update"));
    EventNode.EventPayload.Add(TEXT("ScoreVariable"), TEXT("Chronicle.Variable.Score"));

    FDialogueNode UnreachableNode;
    UnreachableNode.NodeGuid = FGuid::NewGuid();
    UnreachableNode.NodeType = EDialogueNodeType::Speech;

    Tree->Nodes.Add(RootNode);
    Tree->Nodes.Add(SpeechNode);
    Tree->Nodes.Add(ChoiceNode);
    Tree->Nodes.Add(EventNode);
    Tree->Nodes.Add(UnreachableNode);

    FDialogueEdge RootToSpeech;
    RootToSpeech.FromNodeGuid = RootNode.NodeGuid;
    RootToSpeech.ToNodeGuid = SpeechNode.NodeGuid;
    Tree->Edges.Add(RootToSpeech);

    FDialogueEdge SpeechToChoice;
    SpeechToChoice.FromNodeGuid = SpeechNode.NodeGuid;
    SpeechToChoice.ToNodeGuid = ChoiceNode.NodeGuid;
    SpeechToChoice.ConditionExpression = TEXT("Chronicle.Variable.Flag == true");
    Tree->Edges.Add(SpeechToChoice);

    FDialogueEdge ChoiceToEvent;
    ChoiceToEvent.FromNodeGuid = ChoiceNode.NodeGuid;
    ChoiceToEvent.ToNodeGuid = EventNode.NodeGuid;
    Tree->Edges.Add(ChoiceToEvent);

    FDialogueEdge BrokenEdge;
    BrokenEdge.FromNodeGuid = RootNode.NodeGuid;
    BrokenEdge.ToNodeGuid = FGuid::NewGuid();
    Tree->Edges.Add(BrokenEdge);

    FChronicleDialogueAuditReport Report;
    FString Error;
    TestTrue(TEXT("Audit report builds even when validation finds issues"), UChronicleDialogueAuditLibrary::BuildDialogueAuditReport(Tree, Report, Error));
    TestEqual(TEXT("Audit counts nodes"), Report.NodeCount, 5);
    TestEqual(TEXT("Audit counts edges"), Report.EdgeCount, 4);
    TestEqual(TEXT("Audit counts speech lines"), Report.SpeechLineCount, 2);
    TestEqual(TEXT("Audit counts choices"), Report.ChoiceCount, 1);
    TestEqual(TEXT("Audit counts words"), Report.WordCount, 5);
    TestEqual(TEXT("Audit counts broken edges"), Report.BrokenEdgeCount, 1);
    TestEqual(TEXT("Audit counts unreachable nodes"), Report.UnreachableNodeCount, 1);
    TestTrue(TEXT("Audit includes validation errors"), Report.ErrorCount > 0);
    TestTrue(TEXT("Audit includes validation warnings"), Report.WarningCount > 0);

    const FChronicleDialogueSpeakerLineStats* AliceStats = Report.SpeakerLineStats.FindByPredicate([](const FChronicleDialogueSpeakerLineStats& Stats)
    {
        return Stats.SpeakerTag == ChroniclePipelineTests::Tag(TEXT("Chronicle.Speaker.Alice"));
    });
    TestNotNull(TEXT("Audit includes speaker stats"), AliceStats);
    if (AliceStats)
    {
        TestEqual(TEXT("Speaker line count is tracked"), AliceStats->LineCount, 2);
        TestEqual(TEXT("Speaker word count is tracked"), AliceStats->WordCount, 5);
    }

    const FChronicleDialogueVariableUsage* ScoreUsage = Report.VariableUsages.FindByPredicate([](const FChronicleDialogueVariableUsage& Usage)
    {
        return Usage.VariableName == TEXT("Chronicle.Variable.Score");
    });
    TestNotNull(TEXT("Audit includes score variable usage"), ScoreUsage);
    if (ScoreUsage)
    {
        TestEqual(TEXT("Score appears in one condition field"), ScoreUsage->ConditionUsageCount, 1);
        TestEqual(TEXT("Score appears in one event payload"), ScoreUsage->EventPayloadUsageCount, 1);
    }

    FString Json;
    TestTrue(TEXT("Audit report exports as JSON"), UChronicleDialogueAuditLibrary::ExportDialogueAuditReportToJsonString(Report, Json, Error));
    TestTrue(TEXT("Audit JSON includes node count"), Json.Contains(TEXT("NodeCount")));
    TestTrue(TEXT("Audit JSON includes variable usage"), Json.Contains(TEXT("Chronicle.Variable.Score")));

    return true;
}
