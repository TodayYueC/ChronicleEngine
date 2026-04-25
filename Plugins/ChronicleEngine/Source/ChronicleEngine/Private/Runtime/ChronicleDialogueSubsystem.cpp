#include "Runtime/ChronicleDialogueSubsystem.h"

#include "Runtime/DialogueRunner.h"

void UChronicleDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    DialogueRunner = NewObject<UDialogueRunner>(this, TEXT("ChronicleDialogueRunner"));
    DialogueRunner->Initialize(nullptr);
}

void UChronicleDialogueSubsystem::Deinitialize()
{
    DialogueRunner = nullptr;
    Super::Deinitialize();
}

UDialogueRunner* UChronicleDialogueSubsystem::GetDialogueRunner()
{
    if (!DialogueRunner)
    {
        DialogueRunner = NewObject<UDialogueRunner>(this, TEXT("ChronicleDialogueRunner"));
        DialogueRunner->Initialize(nullptr);
    }

    return DialogueRunner;
}

void UChronicleDialogueSubsystem::InitializeDialogueDatabase(UDialogueDatabase* Database)
{
    GetDialogueRunner()->Initialize(Database);
}

