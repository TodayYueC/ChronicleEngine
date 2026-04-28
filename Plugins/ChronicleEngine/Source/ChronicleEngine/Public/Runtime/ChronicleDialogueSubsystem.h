#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChronicleDialogueSubsystem.generated.h"

class UDialogueDatabase;
class UDialogueRunner;
class UDialogueTriggerManager;
class UChronicleDialoguePresentationController;

UCLASS()
class CHRONICLEENGINE_API UChronicleDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    UDialogueRunner* GetDialogueRunner();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    UChronicleDialoguePresentationController* GetPresentationController();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    UDialogueTriggerManager* GetTriggerManager();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void InitializeDialogueDatabase(UDialogueDatabase* Database);

private:
    UPROPERTY()
    TObjectPtr<UDialogueRunner> DialogueRunner;

    UPROPERTY()
    TObjectPtr<UChronicleDialoguePresentationController> PresentationController;

    UPROPERTY()
    TObjectPtr<UDialogueTriggerManager> TriggerManager;
};
