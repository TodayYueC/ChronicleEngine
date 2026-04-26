#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChronicleDialogueDemoActor.generated.h"

class UDialogueTree;

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

    UFUNCTION(BlueprintPure, Category="Chronicle|Sample")
    UDialogueTree* GetRuntimeDemoTree() const { return RuntimeDemoTree; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    bool bStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Sample")
    FName EntryNode;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UDialogueTree> RuntimeDemoTree;
};
