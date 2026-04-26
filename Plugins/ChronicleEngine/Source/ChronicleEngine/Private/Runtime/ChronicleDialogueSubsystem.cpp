#include "Runtime/ChronicleDialogueSubsystem.h"

#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Runtime/DialogueRunner.h"

void UChronicleDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    DialogueRunner = NewObject<UDialogueRunner>(this, TEXT("ChronicleDialogueRunner"));
    DialogueRunner->Initialize(nullptr);
    PresentationController = NewObject<UChronicleDialoguePresentationController>(this, TEXT("ChroniclePresentationController"));
    PresentationController->BindRunner(DialogueRunner);
}

void UChronicleDialogueSubsystem::Deinitialize()
{
    if (PresentationController)
    {
        PresentationController->UnbindRunner();
    }

    PresentationController = nullptr;
    DialogueRunner = nullptr;
    Super::Deinitialize();
}

UDialogueRunner* UChronicleDialogueSubsystem::GetDialogueRunner()
{
    if (!DialogueRunner)
    {
        DialogueRunner = NewObject<UDialogueRunner>(this, TEXT("ChronicleDialogueRunner"));
        DialogueRunner->Initialize(nullptr);
        if (PresentationController)
        {
            PresentationController->BindRunner(DialogueRunner);
        }
    }

    return DialogueRunner;
}

UChronicleDialoguePresentationController* UChronicleDialogueSubsystem::GetPresentationController()
{
    if (!PresentationController)
    {
        PresentationController = NewObject<UChronicleDialoguePresentationController>(this, TEXT("ChroniclePresentationController"));
        PresentationController->BindRunner(GetDialogueRunner());
    }

    return PresentationController;
}

void UChronicleDialogueSubsystem::InitializeDialogueDatabase(UDialogueDatabase* Database)
{
    GetDialogueRunner()->Initialize(Database);
    GetPresentationController()->BindRunner(DialogueRunner);
}
