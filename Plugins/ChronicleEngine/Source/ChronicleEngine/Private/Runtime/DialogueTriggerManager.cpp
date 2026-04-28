#include "Runtime/DialogueTriggerManager.h"

#include "Data/DialogueTrigger.h"
#include "Data/DialogueTree.h"
#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Runtime/DialogueConditionEvaluator.h"
#include "Runtime/DialogueRunner.h"
#include "Runtime/VariableBank.h"

#include "Algo/Sort.h"
#include "HAL/PlatformTime.h"

#define LOCTEXT_NAMESPACE "DialogueTriggerManager"

namespace
{
bool IsBlankTriggerCondition(const FString& Expression)
{
    for (int32 Index = 0; Index < Expression.Len(); ++Index)
    {
        if (!FChar::IsWhitespace(Expression[Index]))
        {
            return false;
        }
    }

    return true;
}
}

void UDialogueTriggerManager::Initialize(UDialogueRunner* InDialogueRunner, UChronicleDialoguePresentationController* InPresentationController)
{
    if (DialogueRunner)
    {
        DialogueRunner->OnDialogueEnded.RemoveDynamic(this, &UDialogueTriggerManager::HandleDialogueEnded);
    }

    DialogueRunner = InDialogueRunner;
    PresentationController = InPresentationController;

    if (DialogueRunner)
    {
        DialogueRunner->OnDialogueEnded.AddUniqueDynamic(this, &UDialogueTriggerManager::HandleDialogueEnded);
    }
}

void UDialogueTriggerManager::Deinitialize()
{
    if (DialogueRunner)
    {
        DialogueRunner->OnDialogueEnded.RemoveDynamic(this, &UDialogueTriggerManager::HandleDialogueEnded);
    }

    DialogueRunner = nullptr;
    PresentationController = nullptr;
    ActiveTrigger = nullptr;
}

void UDialogueTriggerManager::RegisterTrigger(UDialogueTrigger* Trigger)
{
    if (!Trigger)
    {
        return;
    }

    RegisteredTriggers.AddUnique(Trigger);
    Algo::Sort(RegisteredTriggers, [](const TObjectPtr<UDialogueTrigger>& Left, const TObjectPtr<UDialogueTrigger>& Right)
    {
        if (!Left || !Right)
        {
            return Left != nullptr;
        }

        if (Left->Priority == Right->Priority)
        {
            return Left->TriggerTag.ToString() < Right->TriggerTag.ToString();
        }
        return Left->Priority > Right->Priority;
    });
}

bool UDialogueTriggerManager::UnregisterTrigger(UDialogueTrigger* Trigger)
{
    if (!Trigger)
    {
        return false;
    }

    if (ActiveTrigger == Trigger)
    {
        ActiveTrigger = nullptr;
    }

    return RegisteredTriggers.Remove(Trigger) > 0;
}

void UDialogueTriggerManager::ClearTriggers()
{
    RegisteredTriggers.Reset();
    ActiveTrigger = nullptr;
}

void UDialogueTriggerManager::GetRegisteredTriggers(TArray<UDialogueTrigger*>& OutTriggers) const
{
    OutTriggers.Reset();
    OutTriggers.Reserve(RegisteredTriggers.Num());
    for (UDialogueTrigger* Trigger : RegisteredTriggers)
    {
        if (Trigger)
        {
            OutTriggers.Add(Trigger);
        }
    }
}

bool UDialogueTriggerManager::CanActivateTrigger(UDialogueTrigger* Trigger, UObject* Instigator, FText& OutFailureReason) const
{
    if (!Trigger)
    {
        OutFailureReason = LOCTEXT("MissingTrigger", "Trigger is missing.");
        return false;
    }

    if (!DialogueRunner)
    {
        OutFailureReason = LOCTEXT("MissingRunner", "Dialogue runner is missing.");
        return false;
    }

    if (DialogueRunner->IsDialoguePlaying())
    {
        OutFailureReason = LOCTEXT("DialogueAlreadyPlaying", "A dialogue is already playing.");
        return false;
    }

    if (Trigger->bOneShot && IsTriggerConsumed(Trigger->TriggerTag))
    {
        OutFailureReason = LOCTEXT("OneShotConsumed", "Trigger was already consumed.");
        return false;
    }

    if (Trigger->CooldownTime > 0.0f)
    {
        if (const double* LastActivationTime = LastActivationSeconds.Find(Trigger->TriggerTag))
        {
            const double ElapsedSeconds = FPlatformTime::Seconds() - *LastActivationTime;
            if (ElapsedSeconds < static_cast<double>(Trigger->CooldownTime))
            {
                OutFailureReason = FText::Format(
                    LOCTEXT("CooldownActive", "Trigger is cooling down for {0} more seconds."),
                    FText::AsNumber(static_cast<float>(Trigger->CooldownTime - ElapsedSeconds)));
                return false;
            }
        }
    }

    UDialogueTree* TargetTree = ResolveTargetTree(Trigger);
    if (!TargetTree)
    {
        OutFailureReason = LOCTEXT("MissingTargetTree", "Trigger target dialogue tree could not be loaded.");
        return false;
    }

    FGuid EntryGuid;
    if (!TargetTree->ResolveEntryNode(Trigger->EntryNode, EntryGuid))
    {
        OutFailureReason = LOCTEXT("MissingEntryNode", "Trigger entry node could not be resolved.");
        return false;
    }

    return EvaluateActivationConditions(Trigger, OutFailureReason);
}

bool UDialogueTriggerManager::TryActivateTrigger(UDialogueTrigger* Trigger, UObject* Instigator, bool bStartDialogue)
{
    FText FailureReason;
    if (!CanActivateTrigger(Trigger, Instigator, FailureReason))
    {
        RejectTrigger(Trigger, FailureReason);
        return false;
    }

    UDialogueTree* TargetTree = ResolveTargetTree(Trigger);
    if (!TargetTree)
    {
        RejectTrigger(Trigger, LOCTEXT("TargetTreeLost", "Trigger target dialogue tree was unavailable during activation."));
        return false;
    }

    FDialogueTriggerActivation Activation;
    Activation.Trigger = Trigger;
    Activation.TriggerTag = Trigger->TriggerTag;
    Activation.TargetTree = TargetTree;
    Activation.EntryNode = Trigger->EntryNode;
    Activation.TriggerType = Trigger->TriggerType;
    Activation.Priority = Trigger->Priority;
    Activation.Instigator = Instigator;

    ActiveTrigger = bStartDialogue ? Trigger : nullptr;
    LastActivationSeconds.Add(Trigger->TriggerTag, FPlatformTime::Seconds());
    OnTriggerActivated.Broadcast(Activation);

    if (bStartDialogue)
    {
        if (PresentationController)
        {
            PresentationController->StartDialogue(TargetTree, Trigger->EntryNode);
        }
        else
        {
            DialogueRunner->StartDialogue(TargetTree, Trigger->EntryNode);
        }
    }

    return true;
}

bool UDialogueTriggerManager::TryActivateTriggerByTag(FGameplayTag TriggerTag, UObject* Instigator, bool bStartDialogue)
{
    for (UDialogueTrigger* Trigger : RegisteredTriggers)
    {
        if (Trigger && Trigger->TriggerTag == TriggerTag)
        {
            return TryActivateTrigger(Trigger, Instigator, bStartDialogue);
        }
    }

    return false;
}

bool UDialogueTriggerManager::TryActivateBestTrigger(const TArray<UDialogueTrigger*>& CandidateTriggers, EDialogueTriggerType RequiredType, UObject* Instigator, bool bStartDialogue)
{
    UDialogueTrigger* BestTrigger = nullptr;
    for (UDialogueTrigger* Trigger : CandidateTriggers)
    {
        if (!Trigger || Trigger->TriggerType != RequiredType)
        {
            continue;
        }

        FText FailureReason;
        if (!CanActivateTrigger(Trigger, Instigator, FailureReason))
        {
            continue;
        }

        if (!BestTrigger || Trigger->Priority > BestTrigger->Priority)
        {
            BestTrigger = Trigger;
        }
    }

    return BestTrigger ? TryActivateTrigger(BestTrigger, Instigator, bStartDialogue) : false;
}

bool UDialogueTriggerManager::EvaluateAutoTriggers(bool bStartDialogue)
{
    TArray<UDialogueTrigger*> AutoTriggers;
    AutoTriggers.Reserve(RegisteredTriggers.Num());
    for (UDialogueTrigger* Trigger : RegisteredTriggers)
    {
        if (Trigger && Trigger->TriggerType == EDialogueTriggerType::Auto)
        {
            AutoTriggers.Add(Trigger);
        }
    }

    return TryActivateBestTrigger(AutoTriggers, EDialogueTriggerType::Auto, nullptr, bStartDialogue);
}

void UDialogueTriggerManager::SetTriggerConsumed(FGameplayTag TriggerTag, bool bConsumed)
{
    if (!TriggerTag.IsValid())
    {
        return;
    }

    if (bConsumed)
    {
        ConsumedOneShotTriggers.Add(TriggerTag);
    }
    else
    {
        ConsumedOneShotTriggers.Remove(TriggerTag);
    }
}

bool UDialogueTriggerManager::IsTriggerConsumed(FGameplayTag TriggerTag) const
{
    return TriggerTag.IsValid() && ConsumedOneShotTriggers.Contains(TriggerTag);
}

void UDialogueTriggerManager::ResetRuntimeState()
{
    ActiveTrigger = nullptr;
    ConsumedOneShotTriggers.Reset();
    LastActivationSeconds.Reset();
}

void UDialogueTriggerManager::HandleDialogueEnded(UDialogueTree* FinishedTree)
{
    if (ActiveTrigger && ActiveTrigger->bOneShot)
    {
        SetTriggerConsumed(ActiveTrigger->TriggerTag, true);
    }

    ActiveTrigger = nullptr;
}

UDialogueTree* UDialogueTriggerManager::ResolveTargetTree(UDialogueTrigger* Trigger) const
{
    if (!Trigger)
    {
        return nullptr;
    }

    if (UDialogueTree* ExistingTree = Trigger->TargetTree.Get())
    {
        return ExistingTree;
    }

    if (Trigger->TargetTree.IsNull())
    {
        return nullptr;
    }

    return Trigger->TargetTree.LoadSynchronous();
}

bool UDialogueTriggerManager::EvaluateActivationConditions(UDialogueTrigger* Trigger, FText& OutFailureReason) const
{
    if (!Trigger)
    {
        OutFailureReason = LOCTEXT("MissingTriggerForCondition", "Trigger is missing.");
        return false;
    }

    UVariableBank* VariableBank = DialogueRunner ? DialogueRunner->GetVariableBank() : nullptr;
    for (const FString& ConditionExpression : Trigger->ActivationConditions)
    {
        if (IsBlankTriggerCondition(ConditionExpression))
        {
            continue;
        }

        bool bSuccess = false;
        const bool bResult = UDialogueConditionEvaluator::EvaluateCondition(ConditionExpression, VariableBank, bSuccess);
        if (!bSuccess || !bResult)
        {
            OutFailureReason = FText::Format(
                LOCTEXT("ConditionFailed", "Activation condition failed: {0}"),
                FText::FromString(ConditionExpression));
            return false;
        }
    }

    OutFailureReason = FText::GetEmpty();
    return true;
}

void UDialogueTriggerManager::RejectTrigger(UDialogueTrigger* Trigger, const FText& Reason)
{
    OnTriggerRejected.Broadcast(Trigger, Reason);
}

#undef LOCTEXT_NAMESPACE
