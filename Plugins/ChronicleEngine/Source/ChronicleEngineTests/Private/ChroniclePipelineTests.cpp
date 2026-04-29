#include "Misc/AutomationTest.h"

#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"

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
