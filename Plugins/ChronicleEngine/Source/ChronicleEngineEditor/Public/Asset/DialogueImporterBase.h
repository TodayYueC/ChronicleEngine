#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DialogueImporterBase.generated.h"

class UDialogueTree;

UCLASS(Abstract, BlueprintType, EditInlineNew)
class CHRONICLEENGINEEDITOR_API UDialogueImporterBase : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Chronicle|Editor|Import")
    bool ImportFromString(UDialogueTree* Tree, const FString& SourceText, FString& OutError);
    virtual bool ImportFromString_Implementation(UDialogueTree* Tree, const FString& SourceText, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Import")
    bool ImportFromFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="Chronicle|Editor|Import")
    FText GetImporterDisplayName() const;
    virtual FText GetImporterDisplayName_Implementation() const;
};

UCLASS(BlueprintType, EditInlineNew)
class CHRONICLEENGINEEDITOR_API UChronicleCsvDialogueImporter : public UDialogueImporterBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor|Import")
    bool bReplaceExisting = true;

    virtual bool ImportFromString_Implementation(UDialogueTree* Tree, const FString& SourceText, FString& OutError) override;
    virtual FText GetImporterDisplayName_Implementation() const override;
};
