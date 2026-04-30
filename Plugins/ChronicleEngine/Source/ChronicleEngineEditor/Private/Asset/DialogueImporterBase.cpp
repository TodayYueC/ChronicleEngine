#include "Asset/DialogueImporterBase.h"

#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueImporter"

bool UDialogueImporterBase::ImportFromString_Implementation(UDialogueTree* Tree, const FString& SourceText, FString& OutError)
{
    OutError = TEXT("This importer does not implement ImportFromString.");
    return false;
}

bool UDialogueImporterBase::ImportFromFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError)
{
    FString SourceText;
    if (!FFileHelper::LoadFileToString(SourceText, *FilePath))
    {
        OutError = FString::Printf(TEXT("Failed to load import file: %s"), *FilePath);
        return false;
    }

    return ImportFromString(Tree, SourceText, OutError);
}

FText UDialogueImporterBase::GetImporterDisplayName_Implementation() const
{
    return LOCTEXT("BaseImporterDisplayName", "Dialogue Importer");
}

bool UChronicleCsvDialogueImporter::ImportFromString_Implementation(UDialogueTree* Tree, const FString& SourceText, FString& OutError)
{
    return UChronicleDialogueJsonLibrary::ImportDialogueScriptCsvString(Tree, SourceText, bReplaceExisting, OutError);
}

FText UChronicleCsvDialogueImporter::GetImporterDisplayName_Implementation() const
{
    return LOCTEXT("CsvImporterDisplayName", "Chronicle CSV Dialogue Script");
}

#undef LOCTEXT_NAMESPACE
