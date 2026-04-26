#include "Presentation/ChronicleDialoguePresentationController.h"

#include "Data/DialogueTree.h"
#include "Presentation/DialoguePresenter.h"
#include "Runtime/DialogueRunner.h"

UChronicleDialoguePresentationController::UChronicleDialoguePresentationController()
{
}

void UChronicleDialoguePresentationController::BindRunner(UDialogueRunner* InRunner)
{
    if (Runner == InRunner)
    {
        return;
    }

    UnbindRunner();
    Runner = InRunner;

    if (!Runner)
    {
        return;
    }

    Runner->OnDialogueStarted.AddDynamic(this, &UChronicleDialoguePresentationController::HandleDialogueStarted);
    Runner->OnDialogueEnded.AddDynamic(this, &UChronicleDialoguePresentationController::HandleDialogueEnded);
    Runner->OnLineStarted.AddDynamic(this, &UChronicleDialoguePresentationController::HandleLineStarted);
    Runner->OnChoicesPresented.AddDynamic(this, &UChronicleDialoguePresentationController::HandleChoicesPresented);
    Runner->OnDialogueEvent.AddDynamic(this, &UChronicleDialoguePresentationController::HandleDialogueEvent);
    Runner->OnRunnerStateChanged.AddDynamic(this, &UChronicleDialoguePresentationController::HandleRunnerStateChanged);
}

void UChronicleDialoguePresentationController::UnbindRunner()
{
    if (!Runner)
    {
        return;
    }

    Runner->OnDialogueStarted.RemoveAll(this);
    Runner->OnDialogueEnded.RemoveAll(this);
    Runner->OnLineStarted.RemoveAll(this);
    Runner->OnChoicesPresented.RemoveAll(this);
    Runner->OnDialogueEvent.RemoveAll(this);
    Runner->OnRunnerStateChanged.RemoveAll(this);
    Runner = nullptr;
}

bool UChronicleDialoguePresentationController::AddPresenter(UObject* Presenter)
{
    if (!Presenter || !Presenter->GetClass()->ImplementsInterface(UDialoguePresenter::StaticClass()))
    {
        return false;
    }

    for (const TWeakObjectPtr<UObject>& ExistingPresenter : PresenterObjects)
    {
        if (ExistingPresenter.Get() == Presenter)
        {
            return true;
        }
    }

    PresenterObjects.Add(Presenter);
    return true;
}

void UChronicleDialoguePresentationController::RemovePresenter(UObject* Presenter)
{
    PresenterObjects.RemoveAll([Presenter](const TWeakObjectPtr<UObject>& ExistingPresenter)
    {
        return !ExistingPresenter.IsValid() || ExistingPresenter.Get() == Presenter;
    });
}

void UChronicleDialoguePresentationController::StartDialogue(UDialogueTree* Tree, FName EntryNode)
{
    if (!Runner)
    {
        UDialogueRunner* NewRunner = NewObject<UDialogueRunner>(this, TEXT("ChroniclePresentationRunner"));
        NewRunner->Initialize(nullptr);
        BindRunner(NewRunner);
    }

    ClearBacklog();
    PresentedChoices.Reset();
    bHasLastLine = false;
    AutoAdvanceAccumulator = 0.0f;
    Runner->StartDialogue(Tree, EntryNode);
}

void UChronicleDialoguePresentationController::Advance()
{
    if (!Runner)
    {
        return;
    }

    if (Runner->GetRunnerState() == EDialogueRunnerState::WaitingForInput)
    {
        BroadcastLineCompleted();
    }

    AutoAdvanceAccumulator = 0.0f;
    Runner->Advance();
}

void UChronicleDialoguePresentationController::SelectChoice(int32 ChoiceIndex)
{
    if (!Runner)
    {
        return;
    }

    ForEachPresenter([ChoiceIndex](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnChoiceSelected(Presenter, ChoiceIndex);
    });

    PresentedChoices.Reset();
    AutoAdvanceAccumulator = 0.0f;
    Runner->SelectChoice(ChoiceIndex);
}

void UChronicleDialoguePresentationController::EndDialogue()
{
    if (Runner)
    {
        Runner->EndDialogue();
    }
}

void UChronicleDialoguePresentationController::NotifyEventComplete(FGameplayTag EventTag)
{
    if (Runner)
    {
        Runner->NotifyEventComplete(EventTag);
    }
}

void UChronicleDialoguePresentationController::SetAutoAdvanceEnabled(bool bEnabled, float DelaySeconds)
{
    bAutoAdvanceEnabled = bEnabled;
    AutoAdvanceDelay = FMath::Max(0.0f, DelaySeconds);
    AutoAdvanceAccumulator = 0.0f;
    OnAutoAdvanceChanged.Broadcast(bAutoAdvanceEnabled, AutoAdvanceDelay);
}

void UChronicleDialoguePresentationController::SetSkipModeEnabled(bool bEnabled)
{
    bSkipModeEnabled = bEnabled;
    AutoAdvanceAccumulator = 0.0f;
    OnSkipModeChanged.Broadcast(bSkipModeEnabled);
}

void UChronicleDialoguePresentationController::TickPresentation(float DeltaSeconds)
{
    if (!Runner)
    {
        return;
    }

    if (bSkipModeEnabled)
    {
        DrainSkip();
        return;
    }

    if (!bAutoAdvanceEnabled || Runner->GetRunnerState() != EDialogueRunnerState::WaitingForInput)
    {
        return;
    }

    AutoAdvanceAccumulator += FMath::Max(0.0f, DeltaSeconds);
    if (AutoAdvanceAccumulator >= AutoAdvanceDelay)
    {
        Advance();
    }
}

void UChronicleDialoguePresentationController::RequestRollback(int32 Steps)
{
    if (!Runner)
    {
        return;
    }

    const int32 ClampedSteps = FMath::Max(1, Steps);
    Runner->PerformRollback(ClampedSteps);
    SyncBacklogFromRunner();
    AutoAdvanceAccumulator = 0.0f;

    const FGuid CurrentNodeGuid = Runner->GetCurrentNodeGuid();
    OnRollbackPerformed.Broadcast(ClampedSteps, CurrentNodeGuid);
    ForEachPresenter([this](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnRollback(Presenter, Backlog);
    });
}

void UChronicleDialoguePresentationController::ClearBacklog()
{
    Backlog.Reset();
    OnBacklogChanged.Broadcast(Backlog);
}

void UChronicleDialoguePresentationController::HandleDialogueStarted(UDialogueTree* Tree)
{
    PresentedChoices.Reset();
    Backlog.Reset();
    bHasLastLine = false;
    AutoAdvanceAccumulator = 0.0f;

    OnPresentationDialogueStarted.Broadcast(Tree);
    OnBacklogChanged.Broadcast(Backlog);
    ForEachPresenter([Tree](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnDialogueStarted(Presenter, Tree);
    });
}

void UChronicleDialoguePresentationController::HandleDialogueEnded(UDialogueTree* Tree)
{
    PresentedChoices.Reset();
    AutoAdvanceAccumulator = 0.0f;

    OnPresentationDialogueEnded.Broadcast(Tree);
    ForEachPresenter([Tree](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnDialogueEnded(Presenter, Tree);
    });
}

void UChronicleDialoguePresentationController::HandleLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode)
{
    LastLine = Line;
    LastRevealMode = bSkipModeEnabled ? ETextRevealMode::Instant : RevealMode;
    bHasLastLine = true;
    PresentedChoices.Reset();

    FDialogueHistoryEntry Entry;
    Entry.Timestamp = FDateTime::UtcNow();
    Entry.TreeGuid = Runner && Runner->GetCurrentTree() ? Runner->GetCurrentTree()->TreeGuid : FGuid();
    Entry.NodeGuid = Runner ? Runner->GetCurrentNodeGuid() : FGuid();
    Entry.LineID = Line.LineID;
    Entry.SpeakerTag = Line.SpeakerTag;
    Entry.Text = Line.Text;
    Backlog.Add(Entry);

    OnPresentationLineStarted.Broadcast(LastLine, LastRevealMode);
    OnBacklogChanged.Broadcast(Backlog);
    ForEachPresenter([this](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnLineStarted(Presenter, LastLine, LastRevealMode);
    });
}

void UChronicleDialoguePresentationController::HandleChoicesPresented(const TArray<FDialogueChoice>& Choices)
{
    PresentedChoices = Choices;
    AutoAdvanceAccumulator = 0.0f;

    OnPresentationChoicesPresented.Broadcast(PresentedChoices);
    ForEachPresenter([this](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnChoicesPresented(Presenter, PresentedChoices);
    });
}

void UChronicleDialoguePresentationController::HandleDialogueEvent(const FDialogueEventData& EventData)
{
    LastEventData = EventData;
    OnPresentationEvent.Broadcast(EventData);
    ForEachPresenter([&EventData](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnDialogueEvent(Presenter, EventData);
    });
}

void UChronicleDialoguePresentationController::HandleRunnerStateChanged(EDialogueRunnerState OldState, EDialogueRunnerState NewState)
{
    if (NewState != EDialogueRunnerState::WaitingForInput)
    {
        AutoAdvanceAccumulator = 0.0f;
    }

    OnPresentationRunnerStateChanged.Broadcast(OldState, NewState);
    if (NewState == EDialogueRunnerState::WaitingForInput)
    {
        ForEachPresenter([](UObject* Presenter)
        {
            IDialoguePresenter::Execute_OnWaitingForInput(Presenter);
        });
    }
}

void UChronicleDialoguePresentationController::BroadcastLineCompleted()
{
    if (!bHasLastLine)
    {
        return;
    }

    ForEachPresenter([this](UObject* Presenter)
    {
        IDialoguePresenter::Execute_OnLineCompleted(Presenter, LastLine);
    });
}

void UChronicleDialoguePresentationController::DrainSkip()
{
    if (!Runner)
    {
        return;
    }

    const int32 StepLimit = FMath::Max(1, MaxSkipStepsPerTick);
    for (int32 Step = 0; Step < StepLimit && Runner->GetRunnerState() == EDialogueRunnerState::WaitingForInput; ++Step)
    {
        Advance();
    }
}

void UChronicleDialoguePresentationController::SyncBacklogFromRunner()
{
    if (!Runner)
    {
        return;
    }

    FDialogueSaveData Snapshot;
    Runner->SaveState(Snapshot);
    Backlog = Snapshot.History;
    OnBacklogChanged.Broadcast(Backlog);
}

void UChronicleDialoguePresentationController::ForEachPresenter(TFunctionRef<void(UObject*)> Callback)
{
    for (int32 Index = PresenterObjects.Num() - 1; Index >= 0; --Index)
    {
        UObject* Presenter = PresenterObjects[Index].Get();
        if (!Presenter || !Presenter->GetClass()->ImplementsInterface(UDialoguePresenter::StaticClass()))
        {
            PresenterObjects.RemoveAtSwap(Index);
            continue;
        }

        Callback(Presenter);
    }
}
