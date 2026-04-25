#include "Asset/ChronicleDialogueJsonLibrary.h"

#include "Data/DialogueTree.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"

namespace
{
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
}

bool UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetStringField(TEXT("TreeGuid"), Tree->TreeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    RootObject->SetStringField(TEXT("RootNodeGuid"), Tree->RootNodeGuid.ToString(EGuidFormats::DigitsWithHyphens));

    TArray<TSharedPtr<FJsonValue>> NodeValues;
    for (const FDialogueNode& Node : Tree->Nodes)
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
    for (const FDialogueEdge& Edge : Tree->Edges)
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
    for (const FVariableDefinition& Variable : Tree->Variables)
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

    FString Output;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (!FJsonSerializer::Serialize(RootObject, Writer))
    {
        OutError = TEXT("Failed to write JSON.");
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

bool UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    FString Input;
    if (!FFileHelper::LoadFileToString(Input, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to load JSON file: %s"), *FilePath);
        return false;
    }

    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Input);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        OutError = TEXT("Invalid dialogue tree JSON.");
        return false;
    }

    FGuid TreeGuid;
    FGuid RootNodeGuid;
    FGuid::Parse(RootObject->GetStringField(TEXT("TreeGuid")), TreeGuid);
    FGuid::Parse(RootObject->GetStringField(TEXT("RootNodeGuid")), RootNodeGuid);

    TArray<FDialogueNode> Nodes;
    for (const TSharedPtr<FJsonValue>& NodeValue : RootObject->GetArrayField(TEXT("Nodes")))
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
    for (const TSharedPtr<FJsonValue>& EdgeValue : RootObject->GetArrayField(TEXT("Edges")))
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
    for (const TSharedPtr<FJsonValue>& VariableValue : RootObject->GetArrayField(TEXT("Variables")))
    {
        FVariableDefinition Variable;
        if (!JsonValueToStruct(VariableValue, Variable))
        {
            OutError = TEXT("Failed to deserialize a variable definition.");
            return false;
        }
        Variables.Add(Variable);
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

