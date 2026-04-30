#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChronicleDialogueDemoActor.generated.h"

class UDialogueTree;
class UChronicleDialogueWidget;

UCLASS(Blueprintable)
class CHRONICLEENGINE_API AChronicleDialogueDemoActor : public AActor
{
    GENERATED_BODY()

public:
    AChronicleDialogueDemoActor();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Sample")
    UDialogueTree* BuildDemoTree();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Sample")
    void StartDemoDialogue();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Sample")
    UChronicleDialogueWidget* EnsureDemoWidget();

    UFUNCTION(BlueprintPure, Category="Chronicle|Sample")
    UDialogueTree* GetRuntimeDemoTree() const { return RuntimeDemoTree; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Sample")
    UChronicleDialogueWidget* GetRuntimeWidget() const { return RuntimeWidget; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    bool bStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    bool bCreateDefaultWidget = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    TSubclassOf<UChronicleDialogueWidget> DialogueWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    int32 WidgetZOrder = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    FName EntryNode;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UDialogueTree> RuntimeDemoTree;

    UPROPERTY(Transient)
    TObjectPtr<UChronicleDialogueWidget> RuntimeWidget;
};
