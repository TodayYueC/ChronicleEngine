#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Interface.h"
#include "DialoguePresenter.generated.h"

class UDialogueTree;

UINTERFACE(Blueprintable)
class CHRONICLEENGINE_API UDialoguePresenter : public UInterface
{
    GENERATED_BODY()
};

class CHRONICLEENGINE_API IDialoguePresenter
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnDialogueStarted(UDialogueTree* Tree);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnDialogueEnded(UDialogueTree* Tree);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnLineCompleted(const FDialogueLine& Line);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnChoicesPresented(const TArray<FDialogueChoice>& Choices);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnDialogueEvent(const FDialogueEventData& EventData);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnChoiceSelected(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnWaitingForInput();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void OnRollback(const TArray<FDialogueHistoryEntry>& HistorySnapshot);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Chronicle|Presenter")
    void HandleInlineTag(const FGameplayTag& Tag, const FString& Params);
};
