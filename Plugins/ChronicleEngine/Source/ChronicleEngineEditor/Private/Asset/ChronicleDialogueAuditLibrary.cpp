#include "Asset/ChronicleDialogueAuditLibrary.h"

#include "Data/DialogueTree.h"
#include "Dom/JsonObject.h"
#include "GameplayTagsManager.h"
#include "JsonObjectConverter.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
int32 CountWords(const FString& Text)
{
    int32 WordCount = 0;
    bool bInsideWord = false;
    for (int32 Index = 0; Index < Text.Len(); ++Index)
    {
        if (FChar::IsWhitespace(Text[Index]))
        {
            bInsideWord = false;
            continue;
        }

        if (!bInsideWord)
        {
            ++WordCount;
            bInsideWord = true;
        }
    }
    return WordCount;
}

bool IsVariableTokenCharacter(TCHAR Character)
{
    return FChar::IsAlnum(Character) || Character == TEXT('.') || Character == TEXT('_');
}

void ExtractVariableReferences(const FString& Text, TArray<FString>& OutVariableNames)
{
    static const FString Prefix = TEXT("Chronicle.Variable.");
    int32 SearchStart = 0;

    while (SearchStart < Text.Len())
    {
        const int32 PrefixIndex = Text.Find(Prefix, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchStart);
        if (PrefixIndex == INDEX_NONE)
        {
            break;
        }

        int32 EndIndex = PrefixIndex;
        while (EndIndex < Text.Len() && IsVariableTokenCharacter(Text[EndIndex]))
        {
            ++EndIndex;
        }

        FString VariableName = Text.Mid(PrefixIndex, EndIndex - PrefixIndex).TrimStartAndEnd();
        while (VariableName.EndsWith(TEXT(".")))
        {
            VariableName.LeftChopInline(1);
        }

        if (!VariableName.IsEmpty())
        {
            OutVariableNames.AddUnique(VariableName);
        }

        SearchStart = FMath::Max(EndIndex, PrefixIndex + Prefix.Len());
    }
}

FGameplayTag TagFromVariableName(const FString& VariableName)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(FName(*VariableName), false);
}

FChronicleDialogueVariableUsage& FindOrAddVariableUsage(TMap<FString, FChronicleDialogueVariableUsage>& UsageByName, const FString& VariableName)
{
    FChronicleDialogueVariableUsage& Usage = UsageByName.FindOrAdd(VariableName);
    if (Usage.VariableName.IsEmpty())
    {
        Usage.VariableName = VariableName;
        Usage.VariableTag = TagFromVariableName(VariableName);
    }
    return Usage;
}

void AddConditionVariableUsage(TMap<FString, FChronicleDialogueVariableUsage>& UsageByName, const FString& Expression, const FGuid& NodeGuid)
{
    TArray<FString> VariableNames;
    ExtractVariableReferences(Expression, VariableNames);
    for (const FString& VariableName : VariableNames)
    {
        FChronicleDialogueVariableUsage& Usage = FindOrAddVariableUsage(UsageByName, VariableName);
        ++Usage.ConditionUsageCount;
        Usage.NodeGuids.AddUnique(NodeGuid);
    }
}

void AddEventPayloadVariableUsage(TMap<FString, FChronicleDialogueVariableUsage>& UsageByName, const TMap<FName, FString>& Payload, const FGuid& NodeGuid)
{
    for (const TPair<FName, FString>& Pair : Payload)
    {
        TArray<FString> VariableNames;
        ExtractVariableReferences(Pair.Value, VariableNames);
        for (const FString& VariableName : VariableNames)
        {
            FChronicleDialogueVariableUsage& Usage = FindOrAddVariableUsage(UsageByName, VariableName);
            ++Usage.EventPayloadUsageCount;
            Usage.NodeGuids.AddUnique(NodeGuid);
        }
    }
}

void BuildReachabilityStats(const UDialogueTree* Tree, int32& OutBrokenEdgeCount, int32& OutUnreachableNodeCount)
{
    OutBrokenEdgeCount = 0;
    OutUnreachableNodeCount = 0;
    if (!Tree)
    {
        return;
    }

    TSet<FGuid> NodeGuids;
    for (const FDialogueNode& Node : Tree->Nodes)
    {
        if (Node.NodeGuid.IsValid())
        {
            NodeGuids.Add(Node.NodeGuid);
        }
    }

    TMap<FGuid, TArray<FGuid>> Adjacency;
    for (const FDialogueEdge& Edge : Tree->Edges)
    {
        const bool bFromExists = NodeGuids.Contains(Edge.FromNodeGuid);
        const bool bToExists = NodeGuids.Contains(Edge.ToNodeGuid);
        if (!bFromExists || !bToExists)
        {
            ++OutBrokenEdgeCount;
            continue;
        }

        Adjacency.FindOrAdd(Edge.FromNodeGuid).Add(Edge.ToNodeGuid);
    }

    if (!NodeGuids.Contains(Tree->RootNodeGuid))
    {
        OutUnreachableNodeCount = NodeGuids.Num();
        return;
    }

    TSet<FGuid> Visited;
    TArray<FGuid> Stack;
    Stack.Add(Tree->RootNodeGuid);
    while (Stack.Num() > 0)
    {
        const FGuid NodeGuid = Stack.Pop();
        if (Visited.Contains(NodeGuid))
        {
            continue;
        }

        Visited.Add(NodeGuid);
        if (const TArray<FGuid>* Targets = Adjacency.Find(NodeGuid))
        {
            Stack.Append(*Targets);
        }
    }

    for (const FGuid& NodeGuid : NodeGuids)
    {
        if (!Visited.Contains(NodeGuid))
        {
            ++OutUnreachableNodeCount;
        }
    }
}
}

bool UChronicleDialogueAuditLibrary::BuildDialogueAuditReport(UDialogueTree* Tree, FChronicleDialogueAuditReport& OutReport, FString& OutError)
{
    OutReport = FChronicleDialogueAuditReport();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    OutReport.NodeCount = Tree->Nodes.Num();
    OutReport.EdgeCount = Tree->Edges.Num();

    TMap<FGameplayTag, FChronicleDialogueSpeakerLineStats> SpeakerStatsByTag;
    TMap<FString, FChronicleDialogueVariableUsage> VariableUsageByName;

    for (const FDialogueNode& Node : Tree->Nodes)
    {
        AddConditionVariableUsage(VariableUsageByName, Node.ConditionExpression, Node.NodeGuid);

        for (const FDialogueChoice& Choice : Node.Choices)
        {
            ++OutReport.ChoiceCount;
            AddConditionVariableUsage(VariableUsageByName, Choice.VisibilityCondition, Node.NodeGuid);
        }

        if (Node.EventPayload.Num() > 0)
        {
            AddEventPayloadVariableUsage(VariableUsageByName, Node.EventPayload, Node.NodeGuid);
        }

        if (Node.NodeType != EDialogueNodeType::Speech)
        {
            continue;
        }

        for (const FDialogueLine& Line : Node.Lines)
        {
            const int32 LineWordCount = CountWords(Line.Text.ToString());
            ++OutReport.SpeechLineCount;
            OutReport.WordCount += LineWordCount;

            FChronicleDialogueSpeakerLineStats& SpeakerStats = SpeakerStatsByTag.FindOrAdd(Line.SpeakerTag);
            SpeakerStats.SpeakerTag = Line.SpeakerTag;
            ++SpeakerStats.LineCount;
            SpeakerStats.WordCount += LineWordCount;
        }
    }

    for (const FDialogueEdge& Edge : Tree->Edges)
    {
        AddConditionVariableUsage(VariableUsageByName, Edge.ConditionExpression, Edge.FromNodeGuid);
    }

    SpeakerStatsByTag.GenerateValueArray(OutReport.SpeakerLineStats);
    OutReport.SpeakerLineStats.Sort([](const FChronicleDialogueSpeakerLineStats& Left, const FChronicleDialogueSpeakerLineStats& Right)
    {
        return Left.SpeakerTag.ToString() < Right.SpeakerTag.ToString();
    });

    VariableUsageByName.GenerateValueArray(OutReport.VariableUsages);
    OutReport.VariableUsages.Sort([](const FChronicleDialogueVariableUsage& Left, const FChronicleDialogueVariableUsage& Right)
    {
        return Left.VariableName < Right.VariableName;
    });

    BuildReachabilityStats(Tree, OutReport.BrokenEdgeCount, OutReport.UnreachableNodeCount);
    UChronicleDialogueJsonLibrary::ValidateDialogueTree(Tree, OutReport.Issues);
    for (const FChronicleDialogueValidationIssue& Issue : OutReport.Issues)
    {
        if (Issue.Severity == EChronicleDialogueValidationSeverity::Warning)
        {
            ++OutReport.WarningCount;
        }
        else if (Issue.Severity == EChronicleDialogueValidationSeverity::Error)
        {
            ++OutReport.ErrorCount;
        }
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueAuditLibrary::ExportDialogueAuditReportToJsonString(const FChronicleDialogueAuditReport& Report, FString& OutJson, FString& OutError)
{
    TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
    if (!FJsonObjectConverter::UStructToJsonObject(FChronicleDialogueAuditReport::StaticStruct(), &Report, RootObject, 0, 0))
    {
        OutError = TEXT("Failed to serialize dialogue audit report.");
        return false;
    }

    OutJson.Reset();
    const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJson);
    if (!FJsonSerializer::Serialize(RootObject, Writer))
    {
        OutError = TEXT("Failed to write dialogue audit JSON.");
        return false;
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueAuditLibrary::ExportDialogueAuditReportForTreeToJsonString(UDialogueTree* Tree, FString& OutJson, FString& OutError)
{
    FChronicleDialogueAuditReport Report;
    if (!BuildDialogueAuditReport(Tree, Report, OutError))
    {
        return false;
    }

    return ExportDialogueAuditReportToJsonString(Report, OutJson, OutError);
}
