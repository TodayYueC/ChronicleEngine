#include "Misc/AutomationTest.h"

#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Data/DialogueTree.h"
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
