#include "Presentation/ChronicleDialogueChoiceButton.h"

void UChronicleDialogueChoiceButton::InitializeChoice(int32 InChoiceIndex, const FDialogueChoice& InChoice)
{
    ChoiceIndex = InChoiceIndex;
    ChoiceData = InChoice;

    OnClicked.RemoveDynamic(this, &UChronicleDialogueChoiceButton::HandleClicked);
    OnClicked.AddDynamic(this, &UChronicleDialogueChoiceButton::HandleClicked);
}

void UChronicleDialogueChoiceButton::HandleClicked()
{
    OnChoiceButtonClicked.Broadcast(ChoiceIndex);
}
