#include "Runtime/ChronicleDialogueSubsystem.h"

#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Runtime/DialogueRunner.h"
#include "Runtime/DialogueTriggerManager.h"

void UChronicleDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    DialogueRunner = NewObject<UDialogueRunner>(this, TEXT("ChronicleDialogueRunner"));
    DialogueRunner->Initialize(nullptr);
    PresentationController = NewObject<UChronicleDialoguePresentationController>(this, TEXT("ChroniclePresentationController"));
    PresentationController->BindRunner(DialogueRunner);
    TriggerManager = NewObject<UDialogueTriggerManager>(this, TEXT("ChronicleTriggerManager"));
    TriggerManager->Initialize(DialogueRunner, PresentationController);
}

void UChronicleDialogueSubsystem::Deinitialize()
{
    if (TriggerManager)
    {
        TriggerManager->Deinitialize();
    }

    if (PresentationController)
    {
        PresentationController->UnbindRunner();
    }

    TriggerManager = nullptr;
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

UDialogueTriggerManager* UChronicleDialogueSubsystem::GetTriggerManager()
{
    if (!TriggerManager)
    {
        TriggerManager = NewObject<UDialogueTriggerManager>(this, TEXT("ChronicleTriggerManager"));
        TriggerManager->Initialize(GetDialogueRunner(), GetPresentationController());
    }

    return TriggerManager;
}

void UChronicleDialogueSubsystem::InitializeDialogueDatabase(UDialogueDatabase* Database)
{
    GetDialogueRunner()->Initialize(Database);
    GetPresentationController()->BindRunner(DialogueRunner);
    GetTriggerManager()->Initialize(DialogueRunner, PresentationController);
}
