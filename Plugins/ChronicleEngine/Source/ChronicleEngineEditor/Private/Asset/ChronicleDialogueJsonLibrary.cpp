#include "Asset/ChronicleDialogueJsonLibrary.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Dom/JsonObject.h"
#include "GameplayTagsManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
constexpr int32 ChronicleDialogueJsonSchemaVersion = 1;

void AddIssue(TArray<FChronicleDialogueValidationIssue>& Issues, EChronicleDialogueValidationSeverity Severity, const FGuid& NodeGuid, const FString& Message)
{
    FChronicleDialogueValidationIssue Issue;
    Issue.Severity = Severity;
    Issue.NodeGuid = NodeGuid;
    Issue.Message = Message;
    Issues.Add(Issue);
}

template <typename StructType>
bool StructToJsonValue(const StructType& Struct, TSharedPtr<FJsonValue>& OutValue)
{
    TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
    if (!FJsonObjectConverter::UStructToJsonObject(StructType::StaticStruct(), &Struct, Object, 0, 0))
    {
        return false;
    }

    OutValue = MakeShared<FJsonValueObject>(Object);
    return true;
}

template <typename StructType>
bool JsonValueToStruct(const TSharedPtr<FJsonValue>& Value, StructType& OutStruct)
{
    if (!Value.IsValid() || Value->Type != EJson::Object)
    {
        return false;
    }

    return FJsonObjectConverter::JsonObjectToUStruct(Value->AsObject().ToSharedRef(), StructType::StaticStruct(), &OutStruct, 0, 0);
}

FString GuidSortKey(const FGuid& Guid)
{
    return Guid.ToString(EGuidFormats::DigitsWithHyphens);
}

TArray<FDialogueNode> GetSortedNodes(const UDialogueTree* Tree)
{
    TArray<FDialogueNode> Nodes = Tree ? Tree->Nodes : TArray<FDialogueNode>();
    Nodes.Sort([](const FDialogueNode& Left, const FDialogueNode& Right)
    {
        return GuidSortKey(Left.NodeGuid) < GuidSortKey(Right.NodeGuid);
    });
    return Nodes;
}

TArray<FDialogueEdge> GetSortedEdges(const UDialogueTree* Tree)
{
    TArray<FDialogueEdge> Edges = Tree ? Tree->Edges : TArray<FDialogueEdge>();
    Edges.Sort([](const FDialogueEdge& Left, const FDialogueEdge& Right)
    {
        const FString LeftKey = FString::Printf(TEXT("%s:%08d:%s:%08d"),
            *GuidSortKey(Left.FromNodeGuid),
            Left.FromSlotIndex,
            *GuidSortKey(Left.ToNodeGuid),
            Left.ToSlotIndex);
        const FString RightKey = FString::Printf(TEXT("%s:%08d:%s:%08d"),
            *GuidSortKey(Right.FromNodeGuid),
            Right.FromSlotIndex,
            *GuidSortKey(Right.ToNodeGuid),
            Right.ToSlotIndex);
        return LeftKey < RightKey;
    });
    return Edges;
}

FString CsvEscape(const FString& Cell)
{
    FString Escaped = Cell.Replace(TEXT("\""), TEXT("\"\""));
    if (Escaped.Contains(TEXT(",")) || Escaped.Contains(TEXT("\"")) || Escaped.Contains(TEXT("\n")) || Escaped.Contains(TEXT("\r")))
    {
        return FString::Printf(TEXT("\"%s\""), *Escaped);
    }
    return Escaped;
}

void AppendCsvRow(FString& Csv, const TArray<FString>& Cells)
{
    for (int32 Index = 0; Index < Cells.Num(); ++Index)
    {
        if (Index > 0)
        {
            Csv += TEXT(",");
        }
        Csv += CsvEscape(Cells[Index]);
    }
    Csv += LINE_TERMINATOR;
}

bool ParseCsvRows(const FString& Csv, TArray<TArray<FString>>& OutRows, FString& OutError)
{
    OutRows.Reset();
    TArray<FString> CurrentRow;
    FString CurrentCell;
    bool bInQuotes = false;

    for (int32 Index = 0; Index < Csv.Len(); ++Index)
    {
        const TCHAR Char = Csv[Index];
        if (bInQuotes)
        {
            if (Char == TEXT('"'))
            {
                if (Index + 1 < Csv.Len() && Csv[Index + 1] == TEXT('"'))
                {
                    CurrentCell.AppendChar(TEXT('"'));
                    ++Index;
                }
                else
                {
                    bInQuotes = false;
                }
            }
            else
            {
                CurrentCell.AppendChar(Char);
            }
            continue;
        }

        if (Char == TEXT('"'))
        {
            bInQuotes = true;
        }
        else if (Char == TEXT(','))
        {
            CurrentRow.Add(CurrentCell);
            CurrentCell.Reset();
        }
        else if (Char == TEXT('\r') || Char == TEXT('\n'))
        {
            if (Char == TEXT('\r') && Index + 1 < Csv.Len() && Csv[Index + 1] == TEXT('\n'))
            {
                ++Index;
            }
            CurrentRow.Add(CurrentCell);
            CurrentCell.Reset();
            OutRows.Add(CurrentRow);
            CurrentRow.Reset();
        }
        else
        {
            CurrentCell.AppendChar(Char);
        }
    }

    if (bInQuotes)
    {
        OutError = TEXT("CSV ended while a quoted field was still open.");
        return false;
    }

    if (!CurrentCell.IsEmpty() || CurrentRow.Num() > 0)
    {
        CurrentRow.Add(CurrentCell);
        OutRows.Add(CurrentRow);
    }

    OutRows.RemoveAll([](const TArray<FString>& Row)
    {
        return Row.Num() == 0 || (Row.Num() == 1 && Row[0].TrimStartAndEnd().IsEmpty());
    });

    OutError.Reset();
    return true;
}

int32 FindColumn(const TArray<FString>& Header, const FString& Name)
{
    for (int32 Index = 0; Index < Header.Num(); ++Index)
    {
        if (Header[Index].Equals(Name, ESearchCase::IgnoreCase))
        {
            return Index;
        }
    }
    return INDEX_NONE;
}

FString GetCsvCell(const TArray<FString>& Row, int32 Index)
{
    return Row.IsValidIndex(Index) ? Row[Index] : FString();
}

FGameplayTag TagFromString(const FString& TagText)
{
    if (TagText.TrimStartAndEnd().IsEmpty())
    {
        return FGameplayTag();
    }
    return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagText), false);
}

int32 FindColumnAny(const TArray<FString>& Header, const TArray<FString>& Names)
{
    for (const FString& Name : Names)
    {
        const int32 Column = FindColumn(Header, Name);
        if (Column != INDEX_NONE)
        {
            return Column;
        }
    }
    return INDEX_NONE;
}

bool ParseBoolCell(const FString& Cell)
{
    const FString Normalized = Cell.TrimStartAndEnd().ToLower();
    return Normalized == TEXT("true") || Normalized == TEXT("1") || Normalized == TEXT("yes") || Normalized == TEXT("y");
}

void ParseEventPayloadCell(const FString& PayloadCell, TMap<FName, FString>& OutPayload)
{
    OutPayload.Reset();

    TArray<FString> Pairs;
    PayloadCell.ParseIntoArray(Pairs, TEXT(";"), true);
    for (const FString& Pair : Pairs)
    {
        FString Key;
        FString Value;
        if (!Pair.Split(TEXT("="), &Key, &Value))
        {
            continue;
        }

        Key = Key.TrimStartAndEnd();
        Value = Value.TrimStartAndEnd();
        if (!Key.IsEmpty())
        {
            OutPayload.Add(FName(*Key), Value);
        }
    }
}

void AddDialogueEdge(UDialogueTree* Tree, const FGuid& FromGuid, const FGuid& ToGuid, int32 SlotIndex = 0, const FString& ConditionExpression = FString())
{
    FDialogueEdge Edge;
    Edge.FromNodeGuid = FromGuid;
    Edge.ToNodeGuid = ToGuid;
    Edge.FromSlotIndex = SlotIndex;
    Edge.ConditionExpression = ConditionExpression;
    Tree->Edges.Add(Edge);
}

struct FChronicleDialogueScriptCsvRow
{
    FName LineID;
    FText Text;
    FGameplayTag SpeakerTag;
    FGameplayTag EmotionTag;
    FName VoiceID;
    float WaitTime = -1.0f;
    FName NextLineID;
    FString ConditionExpression;
    FGameplayTag EventTag;
    TMap<FName, FString> EventPayload;
    bool bEventIsAsync = false;
    FGuid SpeechNodeGuid;
    FGuid EventNodeGuid;
};

FString GetDefaultDialogueNamespace(const FString& Namespace)
{
    const FString TrimmedNamespace = Namespace.TrimStartAndEnd();
    return TrimmedNamespace.IsEmpty() ? FString(TEXT("Project.Dialogue")) : TrimmedNamespace;
}

FName MakeStableLineId(const UDialogueTree* Tree, const FDialogueNode& Node, int32 LineIndex)
{
    const FString TreeKey = Tree && Tree->TreeGuid.IsValid() ? Tree->TreeGuid.ToString(EGuidFormats::Digits) : FString(TEXT("Tree"));
    const FString NodeKey = Node.NodeGuid.IsValid() ? Node.NodeGuid.ToString(EGuidFormats::Digits) : FString(TEXT("Node"));
    return FName(*FString::Printf(TEXT("DLG_%s_%s_%03d"), *TreeKey.Left(8), *NodeKey.Left(8), LineIndex));
}

FString MakeLocalizationContext(const UDialogueTree* Tree, const FDialogueNode& Node, int32 LineIndex, const FDialogueLine& Line)
{
    return FString::Printf(
        TEXT("TreeGuid=%s; NodeGuid=%s; LineIndex=%d; Speaker=%s; LineID=%s"),
        Tree ? *Tree->TreeGuid.ToString(EGuidFormats::DigitsWithHyphens) : TEXT(""),
        *Node.NodeGuid.ToString(EGuidFormats::DigitsWithHyphens),
        LineIndex,
        *Line.SpeakerTag.ToString(),
        *Line.LineID.ToString());
}

FDialogueLine* FindLineByLocalizationKey(UDialogueTree* Tree, const FString& Key, const FGuid& FallbackNodeGuid, int32 FallbackLineIndex)
{
    if (!Tree)
    {
        return nullptr;
    }

    const FName KeyName(*Key.TrimStartAndEnd());
    if (!KeyName.IsNone())
    {
        for (FDialogueNode& Node : Tree->Nodes)
        {
            for (FDialogueLine& Line : Node.Lines)
            {
                if (Line.LineID == KeyName)
                {
                    return &Line;
                }
            }
        }
    }

    if (FDialogueNode* Node = Tree->FindNodeMutable(FallbackNodeGuid))
    {
        if (Node->Lines.IsValidIndex(FallbackLineIndex))
        {
            return &Node->Lines[FallbackLineIndex];
        }
    }

    return nullptr;
}
}

bool UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonString(UDialogueTree* Tree, FString& OutJson, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetStringField(TEXT("Format"), TEXT("ChronicleDialogueTree"));
    RootObject->SetNumberField(TEXT("SchemaVersion"), ChronicleDialogueJsonSchemaVersion);
    RootObject->SetStringField(TEXT("TreeGuid"), Tree->TreeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    RootObject->SetStringField(TEXT("RootNodeGuid"), Tree->RootNodeGuid.ToString(EGuidFormats::DigitsWithHyphens));

    TArray<TSharedPtr<FJsonValue>> NodeValues;
    for (const FDialogueNode& Node : GetSortedNodes(Tree))
    {
        TSharedPtr<FJsonValue> NodeValue;
        if (!StructToJsonValue(Node, NodeValue))
        {
            OutError = TEXT("Failed to serialize a dialogue node.");
            return false;
        }
        NodeValues.Add(NodeValue);
    }
    RootObject->SetArrayField(TEXT("Nodes"), NodeValues);

    TArray<TSharedPtr<FJsonValue>> EdgeValues;
    for (const FDialogueEdge& Edge : GetSortedEdges(Tree))
    {
        TSharedPtr<FJsonValue> EdgeValue;
        if (!StructToJsonValue(Edge, EdgeValue))
        {
            OutError = TEXT("Failed to serialize a dialogue edge.");
            return false;
        }
        EdgeValues.Add(EdgeValue);
    }
    RootObject->SetArrayField(TEXT("Edges"), EdgeValues);

    TArray<TSharedPtr<FJsonValue>> VariableValues;
    TArray<FVariableDefinition> Variables = Tree->Variables;
    Variables.Sort([](const FVariableDefinition& Left, const FVariableDefinition& Right)
    {
        return Left.VariableTag.ToString() < Right.VariableTag.ToString();
    });
    for (const FVariableDefinition& Variable : Variables)
    {
        TSharedPtr<FJsonValue> VariableValue;
        if (!StructToJsonValue(Variable, VariableValue))
        {
            OutError = TEXT("Failed to serialize a variable definition.");
            return false;
        }
        VariableValues.Add(VariableValue);
    }
    RootObject->SetArrayField(TEXT("Variables"), VariableValues);

    OutJson.Reset();
    const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJson);
    if (!FJsonSerializer::Serialize(RootObject, Writer))
    {
        OutError = TEXT("Failed to write JSON.");
        return false;
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    FString Output;
    if (!ExportDialogueTreeToJsonString(Tree, Output, OutError))
    {
        return false;
    }

    if (!FFileHelper::SaveStringToFile(Output, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to save JSON file: %s"), *FilePath);
        return false;
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonString(UDialogueTree* Tree, const FString& Json, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        OutError = TEXT("Invalid dialogue tree JSON.");
        return false;
    }

    const int32 SchemaVersion = RootObject->GetIntegerField(TEXT("SchemaVersion"));
    if (SchemaVersion != ChronicleDialogueJsonSchemaVersion)
    {
        OutError = FString::Printf(TEXT("Unsupported Chronicle dialogue JSON schema version: %d"), SchemaVersion);
        return false;
    }

    FGuid TreeGuid;
    FGuid RootNodeGuid;
    FGuid::Parse(RootObject->GetStringField(TEXT("TreeGuid")), TreeGuid);
    FGuid::Parse(RootObject->GetStringField(TEXT("RootNodeGuid")), RootNodeGuid);

    const TArray<TSharedPtr<FJsonValue>>* NodeValues = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* EdgeValues = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* VariableValues = nullptr;
    if (!RootObject->TryGetArrayField(TEXT("Nodes"), NodeValues) || !RootObject->TryGetArrayField(TEXT("Edges"), EdgeValues))
    {
        OutError = TEXT("Dialogue JSON must contain Nodes and Edges arrays.");
        return false;
    }

    TArray<FDialogueNode> Nodes;
    for (const TSharedPtr<FJsonValue>& NodeValue : *NodeValues)
    {
        FDialogueNode Node;
        if (!JsonValueToStruct(NodeValue, Node))
        {
            OutError = TEXT("Failed to deserialize a dialogue node.");
            return false;
        }
        Nodes.Add(Node);
    }

    TArray<FDialogueEdge> Edges;
    for (const TSharedPtr<FJsonValue>& EdgeValue : *EdgeValues)
    {
        FDialogueEdge Edge;
        if (!JsonValueToStruct(EdgeValue, Edge))
        {
            OutError = TEXT("Failed to deserialize a dialogue edge.");
            return false;
        }
        Edges.Add(Edge);
    }

    TArray<FVariableDefinition> Variables;
    if (RootObject->TryGetArrayField(TEXT("Variables"), VariableValues))
    {
        for (const TSharedPtr<FJsonValue>& VariableValue : *VariableValues)
        {
            FVariableDefinition Variable;
            if (!JsonValueToStruct(VariableValue, Variable))
            {
                OutError = TEXT("Failed to deserialize a variable definition.");
                return false;
            }
            Variables.Add(Variable);
        }
    }

    Tree->Modify();
    Tree->TreeGuid = TreeGuid.IsValid() ? TreeGuid : FGuid::NewGuid();
    Tree->RootNodeGuid = RootNodeGuid;
    Tree->Nodes = Nodes;
    Tree->Edges = Edges;
    Tree->Variables = Variables;
    Tree->EnsureStableGuids();
    Tree->MarkPackageDirty();

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    FString Input;
    if (!FFileHelper::LoadFileToString(Input, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to load JSON file: %s"), *FilePath);
        return false;
    }

    return ImportDialogueTreeFromJsonString(Tree, Input, OutError);
}

bool UChronicleDialogueJsonLibrary::ExportDialogueLinesToCsvString(UDialogueTree* Tree, FString& OutCsv, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    OutCsv.Reset();
    AppendCsvRow(OutCsv, {
        TEXT("NodeGuid"),
        TEXT("LineIndex"),
        TEXT("LineID"),
        TEXT("SpeakerTag"),
        TEXT("SourceText"),
        TEXT("TranslatedText"),
        TEXT("EmotionTag"),
        TEXT("VoiceID"),
        TEXT("WaitTime"),
        TEXT("ContextComment")
    });

    for (const FDialogueNode& Node : GetSortedNodes(Tree))
    {
        if (Node.NodeType != EDialogueNodeType::Speech)
        {
            continue;
        }

        for (int32 LineIndex = 0; LineIndex < Node.Lines.Num(); ++LineIndex)
        {
            const FDialogueLine& Line = Node.Lines[LineIndex];
            AppendCsvRow(OutCsv, {
                GuidSortKey(Node.NodeGuid),
                FString::FromInt(LineIndex),
                Line.LineID.ToString(),
                Line.SpeakerTag.ToString(),
                Line.Text.ToString(),
                FString(),
                Line.EmotionTag.ToString(),
                Line.VoiceID.ToString(),
                FString::SanitizeFloat(Line.WaitTime),
                FString::Printf(TEXT("NodeType=Speech; NodeGuid=%s; LineIndex=%d"), *GuidSortKey(Node.NodeGuid), LineIndex)
            });
        }
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ExportDialogueLinesToCsvFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    FString Csv;
    if (!ExportDialogueLinesToCsvString(Tree, Csv, OutError))
    {
        return false;
    }

    if (!FFileHelper::SaveStringToFile(Csv, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to save CSV file: %s"), *FilePath);
        return false;
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportDialogueLinesFromCsvString(UDialogueTree* Tree, const FString& Csv, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TArray<TArray<FString>> Rows;
    if (!ParseCsvRows(Csv, Rows, OutError))
    {
        return false;
    }

    if (Rows.Num() < 2)
    {
        OutError = TEXT("CSV must contain a header row and at least one data row.");
        return false;
    }

    const TArray<FString>& Header = Rows[0];
    const int32 NodeGuidColumn = FindColumn(Header, TEXT("NodeGuid"));
    const int32 LineIndexColumn = FindColumn(Header, TEXT("LineIndex"));
    const int32 LineIDColumn = FindColumn(Header, TEXT("LineID"));
    const int32 SpeakerTagColumn = FindColumn(Header, TEXT("SpeakerTag"));
    const int32 SourceTextColumn = FindColumn(Header, TEXT("SourceText"));
    const int32 TranslatedTextColumn = FindColumn(Header, TEXT("TranslatedText"));
    const int32 EmotionTagColumn = FindColumn(Header, TEXT("EmotionTag"));
    const int32 VoiceIDColumn = FindColumn(Header, TEXT("VoiceID"));
    const int32 WaitTimeColumn = FindColumn(Header, TEXT("WaitTime"));

    if (NodeGuidColumn == INDEX_NONE || LineIndexColumn == INDEX_NONE || SourceTextColumn == INDEX_NONE)
    {
        OutError = TEXT("CSV header must include NodeGuid, LineIndex, and SourceText.");
        return false;
    }

    Tree->Modify();
    for (int32 RowIndex = 1; RowIndex < Rows.Num(); ++RowIndex)
    {
        const TArray<FString>& Row = Rows[RowIndex];
        FGuid NodeGuid;
        if (!FGuid::Parse(GetCsvCell(Row, NodeGuidColumn), NodeGuid))
        {
            OutError = FString::Printf(TEXT("Invalid NodeGuid at CSV row %d."), RowIndex + 1);
            return false;
        }

        FDialogueNode* Node = Tree->FindNodeMutable(NodeGuid);
        if (!Node)
        {
            OutError = FString::Printf(TEXT("No node found for CSV row %d."), RowIndex + 1);
            return false;
        }

        const int32 LineIndex = FCString::Atoi(*GetCsvCell(Row, LineIndexColumn));
        if (!Node->Lines.IsValidIndex(LineIndex))
        {
            OutError = FString::Printf(TEXT("Invalid LineIndex at CSV row %d."), RowIndex + 1);
            return false;
        }

        FDialogueLine& Line = Node->Lines[LineIndex];
        const FString TranslatedText = GetCsvCell(Row, TranslatedTextColumn);
        const FString SourceText = GetCsvCell(Row, SourceTextColumn);
        Line.Text = FText::FromString(TranslatedText.IsEmpty() ? SourceText : TranslatedText);

        if (LineIDColumn != INDEX_NONE && !GetCsvCell(Row, LineIDColumn).IsEmpty())
        {
            Line.LineID = FName(*GetCsvCell(Row, LineIDColumn));
        }
        if (SpeakerTagColumn != INDEX_NONE)
        {
            Line.SpeakerTag = TagFromString(GetCsvCell(Row, SpeakerTagColumn));
        }
        if (EmotionTagColumn != INDEX_NONE)
        {
            Line.EmotionTag = TagFromString(GetCsvCell(Row, EmotionTagColumn));
        }
        if (VoiceIDColumn != INDEX_NONE)
        {
            Line.VoiceID = FName(*GetCsvCell(Row, VoiceIDColumn));
        }
        if (WaitTimeColumn != INDEX_NONE && !GetCsvCell(Row, WaitTimeColumn).IsEmpty())
        {
            Line.WaitTime = FCString::Atof(*GetCsvCell(Row, WaitTimeColumn));
        }
    }

    Tree->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportDialogueLinesFromCsvFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    FString Csv;
    if (!FFileHelper::LoadFileToString(Csv, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to load CSV file: %s"), *FilePath);
        return false;
    }

    return ImportDialogueLinesFromCsvString(Tree, Csv, OutError);
}

bool UChronicleDialogueJsonLibrary::ImportDialogueScriptCsvString(UDialogueTree* Tree, const FString& Csv, bool bReplaceExisting, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TArray<TArray<FString>> Rows;
    if (!ParseCsvRows(Csv, Rows, OutError))
    {
        return false;
    }

    if (Rows.Num() < 2)
    {
        OutError = TEXT("Dialogue script CSV must contain a header row and at least one data row.");
        return false;
    }

    const TArray<FString>& Header = Rows[0];
    const int32 LineIdColumn = FindColumn(Header, TEXT("LineID"));
    const int32 TextColumn = FindColumn(Header, TEXT("Text"));
    const int32 SpeakerTagColumn = FindColumnAny(Header, { TEXT("SpeakerTag"), TEXT("Speaker") });
    const int32 EmotionTagColumn = FindColumn(Header, TEXT("EmotionTag"));
    const int32 VoiceIdColumn = FindColumn(Header, TEXT("VoiceID"));
    const int32 WaitTimeColumn = FindColumn(Header, TEXT("WaitTime"));
    const int32 NextLineIdColumn = FindColumnAny(Header, { TEXT("NextLineID"), TEXT("NextLine"), TEXT("TargetLineID") });
    const int32 ConditionColumn = FindColumnAny(Header, { TEXT("ConditionExpression"), TEXT("Condition") });
    const int32 EventTagColumn = FindColumn(Header, TEXT("EventTag"));
    const int32 EventPayloadColumn = FindColumnAny(Header, { TEXT("EventPayload"), TEXT("Payload") });
    const int32 EventAsyncColumn = FindColumnAny(Header, { TEXT("bEventIsAsync"), TEXT("EventAsync"), TEXT("Async") });

    if (LineIdColumn == INDEX_NONE || TextColumn == INDEX_NONE)
    {
        OutError = TEXT("Dialogue script CSV header must include LineID and Text.");
        return false;
    }

    TArray<FChronicleDialogueScriptCsvRow> ImportedRows;
    ImportedRows.Reserve(Rows.Num() - 1);
    TSet<FName> ImportedLineIds;

    for (int32 RowIndex = 1; RowIndex < Rows.Num(); ++RowIndex)
    {
        const TArray<FString>& Row = Rows[RowIndex];
        const FString LineIdText = GetCsvCell(Row, LineIdColumn).TrimStartAndEnd();
        if (LineIdText.IsEmpty())
        {
            OutError = FString::Printf(TEXT("Dialogue script CSV row %d has no LineID."), RowIndex + 1);
            return false;
        }

        FChronicleDialogueScriptCsvRow ImportedRow;
        ImportedRow.LineID = FName(*LineIdText);
        if (ImportedLineIds.Contains(ImportedRow.LineID))
        {
            OutError = FString::Printf(TEXT("Duplicate LineID in dialogue script CSV: %s"), *ImportedRow.LineID.ToString());
            return false;
        }
        ImportedLineIds.Add(ImportedRow.LineID);

        ImportedRow.Text = FText::FromString(GetCsvCell(Row, TextColumn));
        ImportedRow.SpeakerTag = SpeakerTagColumn != INDEX_NONE ? TagFromString(GetCsvCell(Row, SpeakerTagColumn)) : FGameplayTag();
        ImportedRow.EmotionTag = EmotionTagColumn != INDEX_NONE ? TagFromString(GetCsvCell(Row, EmotionTagColumn)) : FGameplayTag();
        ImportedRow.VoiceID = VoiceIdColumn != INDEX_NONE ? FName(*GetCsvCell(Row, VoiceIdColumn).TrimStartAndEnd()) : NAME_None;
        if (WaitTimeColumn != INDEX_NONE && !GetCsvCell(Row, WaitTimeColumn).TrimStartAndEnd().IsEmpty())
        {
            ImportedRow.WaitTime = FCString::Atof(*GetCsvCell(Row, WaitTimeColumn));
        }
        ImportedRow.NextLineID = NextLineIdColumn != INDEX_NONE ? FName(*GetCsvCell(Row, NextLineIdColumn).TrimStartAndEnd()) : NAME_None;
        ImportedRow.ConditionExpression = ConditionColumn != INDEX_NONE ? GetCsvCell(Row, ConditionColumn).TrimStartAndEnd() : FString();
        ImportedRow.EventTag = EventTagColumn != INDEX_NONE ? TagFromString(GetCsvCell(Row, EventTagColumn)) : FGameplayTag();
        if (EventTagColumn != INDEX_NONE && !GetCsvCell(Row, EventTagColumn).TrimStartAndEnd().IsEmpty() && !ImportedRow.EventTag.IsValid())
        {
            OutError = FString::Printf(TEXT("Invalid EventTag at CSV row %d: %s"), RowIndex + 1, *GetCsvCell(Row, EventTagColumn));
            return false;
        }
        if (EventPayloadColumn != INDEX_NONE)
        {
            ParseEventPayloadCell(GetCsvCell(Row, EventPayloadColumn), ImportedRow.EventPayload);
        }
        ImportedRow.bEventIsAsync = EventAsyncColumn != INDEX_NONE && ParseBoolCell(GetCsvCell(Row, EventAsyncColumn));

        ImportedRows.Add(ImportedRow);
    }

    if (ImportedRows.Num() == 0)
    {
        OutError = TEXT("Dialogue script CSV did not contain any importable rows.");
        return false;
    }

    TMap<FName, FGuid> LineIdToNodeGuid;
    if (!bReplaceExisting)
    {
        for (const FDialogueNode& Node : Tree->Nodes)
        {
            for (const FDialogueLine& Line : Node.Lines)
            {
                if (!Line.LineID.IsNone())
                {
                    LineIdToNodeGuid.FindOrAdd(Line.LineID, Node.NodeGuid);
                }
            }
        }

        for (const FName& ImportedLineID : ImportedLineIds)
        {
            if (LineIdToNodeGuid.Contains(ImportedLineID))
            {
                OutError = FString::Printf(TEXT("Imported LineID already exists in the target tree: %s"), *ImportedLineID.ToString());
                return false;
            }
        }
    }

    for (const FChronicleDialogueScriptCsvRow& ImportedRow : ImportedRows)
    {
        if (ImportedRow.NextLineID.IsNone())
        {
            continue;
        }

        if (!ImportedLineIds.Contains(ImportedRow.NextLineID) && !LineIdToNodeGuid.Contains(ImportedRow.NextLineID))
        {
            OutError = FString::Printf(TEXT("NextLineID '%s' does not resolve to an imported or existing line."), *ImportedRow.NextLineID.ToString());
            return false;
        }
    }

    Tree->Modify();
    if (bReplaceExisting)
    {
        Tree->Nodes.Reset();
        Tree->Edges.Reset();
        Tree->EditorStates.Reset();
        Tree->RootNodeGuid.Invalidate();
    }

    if (!Tree->TreeGuid.IsValid())
    {
        Tree->TreeGuid = FGuid::NewGuid();
    }

    if (!Tree->RootNodeGuid.IsValid() || !Tree->FindNode(Tree->RootNodeGuid))
    {
        FDialogueNode RootNode;
        RootNode.NodeGuid = FGuid::NewGuid();
        RootNode.NodeType = EDialogueNodeType::Root;
        Tree->RootNodeGuid = RootNode.NodeGuid;
        Tree->Nodes.Add(RootNode);
    }

    for (FChronicleDialogueScriptCsvRow& ImportedRow : ImportedRows)
    {
        FDialogueNode SpeechNode;
        SpeechNode.NodeGuid = FGuid::NewGuid();
        SpeechNode.NodeType = EDialogueNodeType::Speech;

        FDialogueLine Line;
        Line.LineID = ImportedRow.LineID;
        Line.Text = ImportedRow.Text;
        Line.SpeakerTag = ImportedRow.SpeakerTag;
        Line.EmotionTag = ImportedRow.EmotionTag;
        Line.VoiceID = ImportedRow.VoiceID;
        Line.WaitTime = ImportedRow.WaitTime;
        SpeechNode.Lines.Add(Line);

        ImportedRow.SpeechNodeGuid = SpeechNode.NodeGuid;
        LineIdToNodeGuid.Add(ImportedRow.LineID, ImportedRow.SpeechNodeGuid);
        Tree->Nodes.Add(SpeechNode);

        if (ImportedRow.EventTag.IsValid())
        {
            FDialogueNode EventNode;
            EventNode.NodeGuid = FGuid::NewGuid();
            EventNode.NodeType = EDialogueNodeType::Event;
            EventNode.EventTag = ImportedRow.EventTag;
            EventNode.EventPayload = ImportedRow.EventPayload;
            EventNode.bEventIsAsync = ImportedRow.bEventIsAsync;
            ImportedRow.EventNodeGuid = EventNode.NodeGuid;
            Tree->Nodes.Add(EventNode);
        }
    }

    AddDialogueEdge(Tree, Tree->RootNodeGuid, ImportedRows[0].SpeechNodeGuid);

    for (const FChronicleDialogueScriptCsvRow& ImportedRow : ImportedRows)
    {
        FGuid SourceGuid = ImportedRow.SpeechNodeGuid;
        FString ConditionExpression = ImportedRow.ConditionExpression;

        if (ImportedRow.EventNodeGuid.IsValid())
        {
            AddDialogueEdge(Tree, ImportedRow.SpeechNodeGuid, ImportedRow.EventNodeGuid, 0, ConditionExpression);
            SourceGuid = ImportedRow.EventNodeGuid;
            ConditionExpression.Reset();
        }

        if (ImportedRow.NextLineID.IsNone())
        {
            continue;
        }

        const FGuid* TargetGuid = LineIdToNodeGuid.Find(ImportedRow.NextLineID);
        if (!TargetGuid || !TargetGuid->IsValid())
        {
            OutError = FString::Printf(TEXT("NextLineID '%s' does not resolve to an imported or existing line."), *ImportedRow.NextLineID.ToString());
            return false;
        }

        AddDialogueEdge(Tree, SourceGuid, *TargetGuid, 0, ConditionExpression);
    }

    Tree->EnsureStableGuids();
    Tree->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportDialogueScriptCsvFile(UDialogueTree* Tree, const FString& FilePath, bool bReplaceExisting, FString& OutError)
{
    FString Csv;
    if (!FFileHelper::LoadFileToString(Csv, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to load dialogue script CSV file: %s"), *FilePath);
        return false;
    }

    return ImportDialogueScriptCsvString(Tree, Csv, bReplaceExisting, OutError);
}

bool UChronicleDialogueJsonLibrary::EnsureStableLineIds(UDialogueTree* Tree, int32& OutUpdatedCount, FString& OutError)
{
    OutUpdatedCount = 0;
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    Tree->EnsureStableGuids();
    TSet<FName> SeenLineIds;
    Tree->Modify();

    for (FDialogueNode& Node : Tree->Nodes)
    {
        for (int32 LineIndex = 0; LineIndex < Node.Lines.Num(); ++LineIndex)
        {
            FDialogueLine& Line = Node.Lines[LineIndex];
            if (!Line.LineID.IsNone() && !SeenLineIds.Contains(Line.LineID))
            {
                SeenLineIds.Add(Line.LineID);
                continue;
            }

            Line.LineID = MakeStableLineId(Tree, Node, LineIndex);
            SeenLineIds.Add(Line.LineID);
            ++OutUpdatedCount;
        }
    }

    if (OutUpdatedCount > 0)
    {
        Tree->MarkPackageDirty();
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::GatherDialogueTextsFromTree(UDialogueTree* Tree, const FString& Namespace, const FString& Culture, TArray<FChronicleDialogueLocalizationEntry>& OutEntries, FString& OutError)
{
    OutEntries.Reset();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    int32 UpdatedLineIds = 0;
    if (!EnsureStableLineIds(Tree, UpdatedLineIds, OutError))
    {
        return false;
    }

    const FString StableNamespace = GetDefaultDialogueNamespace(Namespace);
    const FString StableCulture = Culture.TrimStartAndEnd();
    for (const FDialogueNode& Node : GetSortedNodes(Tree))
    {
        if (Node.NodeType != EDialogueNodeType::Speech)
        {
            continue;
        }

        for (int32 LineIndex = 0; LineIndex < Node.Lines.Num(); ++LineIndex)
        {
            const FDialogueLine& Line = Node.Lines[LineIndex];

            FChronicleDialogueLocalizationEntry Entry;
            Entry.Namespace = StableNamespace;
            Entry.Key = Line.LineID;
            Entry.Culture = StableCulture;
            Entry.TreeGuid = Tree->TreeGuid;
            Entry.NodeGuid = Node.NodeGuid;
            Entry.LineIndex = LineIndex;
            Entry.LineID = Line.LineID;
            Entry.SpeakerTag = Line.SpeakerTag;
            Entry.SourceText = Line.Text;
            Entry.ContextComment = MakeLocalizationContext(Tree, Node, LineIndex, Line);
            OutEntries.Add(Entry);
        }
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::GatherDialogueTextsFromDatabase(UDialogueDatabase* Database, const FString& Culture, TArray<FChronicleDialogueLocalizationEntry>& OutEntries, FString& OutError)
{
    OutEntries.Reset();
    if (!Database)
    {
        OutError = TEXT("No dialogue database supplied.");
        return false;
    }

    const FString Namespace = GetDefaultDialogueNamespace(Database->LocalizationSettings.Namespace);
    for (const TSoftObjectPtr<UDialogueTree>& TreePtr : Database->DialogueTrees)
    {
        UDialogueTree* Tree = TreePtr.Get();
        if (!Tree && !TreePtr.IsNull())
        {
            Tree = TreePtr.LoadSynchronous();
        }

        if (!Tree)
        {
            continue;
        }

        TArray<FChronicleDialogueLocalizationEntry> TreeEntries;
        if (!GatherDialogueTextsFromTree(Tree, Namespace, Culture, TreeEntries, OutError))
        {
            return false;
        }

        OutEntries.Append(TreeEntries);
    }

    OutEntries.Sort([](const FChronicleDialogueLocalizationEntry& Left, const FChronicleDialogueLocalizationEntry& Right)
    {
        if (Left.Namespace != Right.Namespace)
        {
            return Left.Namespace < Right.Namespace;
        }
        return Left.Key.LexicalLess(Right.Key);
    });

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ExportLocalizationCsvFromTree(UDialogueTree* Tree, const FString& Namespace, const FString& Culture, FString& OutCsv, FString& OutError)
{
    TArray<FChronicleDialogueLocalizationEntry> Entries;
    if (!GatherDialogueTextsFromTree(Tree, Namespace, Culture, Entries, OutError))
    {
        return false;
    }

    OutCsv.Reset();
    AppendCsvRow(OutCsv, {
        TEXT("Namespace"),
        TEXT("Key"),
        TEXT("Culture"),
        TEXT("TreeGuid"),
        TEXT("NodeGuid"),
        TEXT("LineIndex"),
        TEXT("LineID"),
        TEXT("SpeakerTag"),
        TEXT("SourceText"),
        TEXT("TranslatedText"),
        TEXT("ContextComment")
    });

    for (const FChronicleDialogueLocalizationEntry& Entry : Entries)
    {
        AppendCsvRow(OutCsv, {
            Entry.Namespace,
            Entry.Key.ToString(),
            Entry.Culture,
            Entry.TreeGuid.ToString(EGuidFormats::DigitsWithHyphens),
            Entry.NodeGuid.ToString(EGuidFormats::DigitsWithHyphens),
            FString::FromInt(Entry.LineIndex),
            Entry.LineID.ToString(),
            Entry.SpeakerTag.ToString(),
            Entry.SourceText.ToString(),
            Entry.TranslatedText.ToString(),
            Entry.ContextComment
        });
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ImportLocalizationCsvToTree(UDialogueTree* Tree, const FString& Csv, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TArray<TArray<FString>> Rows;
    if (!ParseCsvRows(Csv, Rows, OutError))
    {
        return false;
    }

    if (Rows.Num() < 2)
    {
        OutError = TEXT("Localization CSV must contain a header row and at least one data row.");
        return false;
    }

    const TArray<FString>& Header = Rows[0];
    const int32 KeyColumn = FindColumn(Header, TEXT("Key"));
    const int32 LineIdColumn = FindColumn(Header, TEXT("LineID"));
    const int32 NodeGuidColumn = FindColumn(Header, TEXT("NodeGuid"));
    const int32 LineIndexColumn = FindColumn(Header, TEXT("LineIndex"));
    const int32 TranslatedTextColumn = FindColumn(Header, TEXT("TranslatedText"));

    if (TranslatedTextColumn == INDEX_NONE || (KeyColumn == INDEX_NONE && LineIdColumn == INDEX_NONE))
    {
        OutError = TEXT("Localization CSV header must include TranslatedText and either Key or LineID.");
        return false;
    }

    Tree->Modify();
    bool bImportedAnyText = false;
    for (int32 RowIndex = 1; RowIndex < Rows.Num(); ++RowIndex)
    {
        const TArray<FString>& Row = Rows[RowIndex];
        const FString TranslatedText = GetCsvCell(Row, TranslatedTextColumn);
        if (TranslatedText.IsEmpty())
        {
            continue;
        }

        FString Key = KeyColumn != INDEX_NONE ? GetCsvCell(Row, KeyColumn) : FString();
        if (Key.IsEmpty() && LineIdColumn != INDEX_NONE)
        {
            Key = GetCsvCell(Row, LineIdColumn);
        }

        FGuid NodeGuid;
        if (NodeGuidColumn != INDEX_NONE)
        {
            FGuid::Parse(GetCsvCell(Row, NodeGuidColumn), NodeGuid);
        }
        const int32 LineIndex = LineIndexColumn != INDEX_NONE ? FCString::Atoi(*GetCsvCell(Row, LineIndexColumn)) : INDEX_NONE;

        FDialogueLine* Line = FindLineByLocalizationKey(Tree, Key, NodeGuid, LineIndex);
        if (!Line)
        {
            OutError = FString::Printf(TEXT("No dialogue line found for localization CSV row %d."), RowIndex + 1);
            return false;
        }

        Line->Text = FText::FromString(TranslatedText);
        bImportedAnyText = true;
    }

    if (bImportedAnyText)
    {
        Tree->MarkPackageDirty();
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueJsonLibrary::ValidateDialogueTree(UDialogueTree* Tree, TArray<FChronicleDialogueValidationIssue>& OutIssues)
{
    OutIssues.Reset();
    if (!Tree)
    {
        AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, FGuid(), TEXT("No dialogue tree supplied."));
        return false;
    }

    if (!Tree->TreeGuid.IsValid())
    {
        AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, FGuid(), TEXT("Dialogue tree has no valid TreeGuid."));
    }

    TSet<FGuid> NodeGuids;
    for (const FDialogueNode& Node : Tree->Nodes)
    {
        if (!Node.NodeGuid.IsValid())
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, FGuid(), TEXT("A dialogue node has an invalid GUID."));
            continue;
        }

        if (NodeGuids.Contains(Node.NodeGuid))
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, Node.NodeGuid, TEXT("Duplicate dialogue node GUID."));
        }
        NodeGuids.Add(Node.NodeGuid);

        if (Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() == 0)
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Warning, Node.NodeGuid, TEXT("Speech node has no lines."));
        }
        if (Node.NodeType == EDialogueNodeType::Choice && Node.Choices.Num() == 0)
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Warning, Node.NodeGuid, TEXT("Choice node has no choices."));
        }
        if (Node.NodeType == EDialogueNodeType::Event && !Node.EventTag.IsValid())
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Warning, Node.NodeGuid, TEXT("Event node has no valid EventTag."));
        }
    }

    if (!Tree->RootNodeGuid.IsValid() || !NodeGuids.Contains(Tree->RootNodeGuid))
    {
        AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, Tree->RootNodeGuid, TEXT("RootNodeGuid does not point to an existing node."));
    }

    TMap<FGuid, TArray<FGuid>> Adjacency;
    for (const FDialogueEdge& Edge : Tree->Edges)
    {
        if (!NodeGuids.Contains(Edge.FromNodeGuid))
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, Edge.FromNodeGuid, TEXT("Edge starts from a missing node."));
        }
        if (!NodeGuids.Contains(Edge.ToNodeGuid))
        {
            AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Error, Edge.ToNodeGuid, TEXT("Edge points to a missing node."));
        }

        if (NodeGuids.Contains(Edge.FromNodeGuid) && NodeGuids.Contains(Edge.ToNodeGuid))
        {
            Adjacency.FindOrAdd(Edge.FromNodeGuid).Add(Edge.ToNodeGuid);
        }
    }

    if (NodeGuids.Contains(Tree->RootNodeGuid))
    {
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
                AddIssue(OutIssues, EChronicleDialogueValidationSeverity::Warning, NodeGuid, TEXT("Node is unreachable from the root node."));
            }
        }
    }

    return !OutIssues.ContainsByPredicate([](const FChronicleDialogueValidationIssue& Issue)
    {
        return Issue.Severity == EChronicleDialogueValidationSeverity::Error;
    });
}
