#include "Samples/ChronicleDialogueDemoActor.h"

#include "Data/DialogueTree.h"
#include "Engine/GameInstance.h"
#include "GameplayTagsManager.h"
#include "Presentation/ChronicleDialogueDefaultWidget.h"
#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Presentation/ChronicleDialogueWidget.h"
#include "Runtime/ChronicleDialogueSubsystem.h"
#include "Runtime/DialogueRunner.h"

namespace
{
FGameplayTag ChronicleTag(const TCHAR* TagName)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(FName(TagName), false);
}

FGuid AddDemoNode(UDialogueTree* Tree, EDialogueNodeType NodeType, const FVector2D& Position)
{
    FDialogueNode Node;
    Node.NodeGuid = FGuid::NewGuid();
    Node.NodeType = NodeType;
    Node.Position = Position;
    Tree->Nodes.Add(Node);
    return Node.NodeGuid;
}

void AddDemoEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 SlotIndex = 0)
{
    FDialogueEdge Edge;
    Edge.FromNodeGuid = FromNodeGuid;
    Edge.ToNodeGuid = ToNodeGuid;
    Edge.FromSlotIndex = SlotIndex;
    Tree->Edges.Add(Edge);
}

FDialogueLine MakeDemoLine(const TCHAR* LineID, const TCHAR* Text, const TCHAR* VoiceID = TEXT(""))
{
    FDialogueLine Line;
    Line.LineID = FName(LineID);
    Line.SpeakerTag = ChronicleTag(TEXT("Chronicle.Speaker.Alice"));
    Line.Text = FText::FromString(Text);
    Line.VoiceID = FName(VoiceID);
    return Line;
}
}

AChronicleDialogueDemoActor::AChronicleDialogueDemoActor()
{
    PrimaryActorTick.bCanEverTick = false;
    DialogueWidgetClass = UChronicleDialogueDefaultWidget::StaticClass();
}

void AChronicleDialogueDemoActor::BeginPlay()
{
    Super::BeginPlay();

    if (bStartOnBeginPlay)
    {
        StartDemoDialogue();
    }
}

UDialogueTree* AChronicleDialogueDemoActor::BuildDemoTree()
{
    RuntimeDemoTree = NewObject<UDialogueTree>(this, TEXT("ChronicleRuntimeDemoTree"));
    RuntimeDemoTree->TreeGuid = FGuid::NewGuid();

    const FGuid RootGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Root, FVector2D(0.0f, 0.0f));
    const FGuid CameraGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Event, FVector2D(280.0f, 0.0f));
    const FGuid IntroGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Speech, FVector2D(560.0f, 0.0f));
    const FGuid ChoiceGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Choice, FVector2D(840.0f, 0.0f));
    const FGuid YesGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Speech, FVector2D(1120.0f, -120.0f));
    const FGuid NoGuid = AddDemoNode(RuntimeDemoTree, EDialogueNodeType::Speech, FVector2D(1120.0f, 120.0f));
    RuntimeDemoTree->RootNodeGuid = RootGuid;

    FDialogueNode* CameraNode = RuntimeDemoTree->FindNodeMutable(CameraGuid);
    CameraNode->EventTag = ChronicleTag(TEXT("Chronicle.Camera.Cut"));
    CameraNode->EventPayload.Add(TEXT("Shot"), TEXT("ChronicleDemoIntro"));
    CameraNode->EventPayload.Add(TEXT("BlendTime"), TEXT("0.25"));

    FDialogueNode* IntroNode = RuntimeDemoTree->FindNodeMutable(IntroGuid);
    IntroNode->Lines.Add(MakeDemoLine(TEXT("Demo_Intro_001"), TEXT("Welcome to Chronicle Engine."), TEXT("VO_Demo_Intro_001")));
    IntroNode->Lines.Add(MakeDemoLine(TEXT("Demo_Intro_002"), TEXT("Auto, skip, backlog, rollback, camera, and voice hooks are all routed through the presentation controller."), TEXT("VO_Demo_Intro_002")));

    FDialogueNode* ChoiceNode = RuntimeDemoTree->FindNodeMutable(ChoiceGuid);
    FDialogueChoice ContinueChoice;
    ContinueChoice.Text = FText::FromString(TEXT("Show me the optimistic line."));
    ChoiceNode->Choices.Add(ContinueChoice);

    FDialogueChoice LaterChoice;
    LaterChoice.Text = FText::FromString(TEXT("Keep it short."));
    ChoiceNode->Choices.Add(LaterChoice);

    RuntimeDemoTree->FindNodeMutable(YesGuid)->Lines.Add(MakeDemoLine(TEXT("Demo_Yes_001"), TEXT("Then we keep building from a playable slice."), TEXT("VO_Demo_Yes_001")));
    RuntimeDemoTree->FindNodeMutable(NoGuid)->Lines.Add(MakeDemoLine(TEXT("Demo_No_001"), TEXT("Short version: it works, and the UI can stay in Blueprint."), TEXT("VO_Demo_No_001")));

    AddDemoEdge(RuntimeDemoTree, RootGuid, CameraGuid);
    AddDemoEdge(RuntimeDemoTree, CameraGuid, IntroGuid);
    AddDemoEdge(RuntimeDemoTree, IntroGuid, ChoiceGuid);
    AddDemoEdge(RuntimeDemoTree, ChoiceGuid, YesGuid, 0);
    AddDemoEdge(RuntimeDemoTree, ChoiceGuid, NoGuid, 1);

    return RuntimeDemoTree;
}

void AChronicleDialogueDemoActor::StartDemoDialogue()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return;
    }

    UChronicleDialogueSubsystem* DialogueSubsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
    if (!DialogueSubsystem)
    {
        return;
    }

    UDialogueTree* DemoTree = RuntimeDemoTree ? RuntimeDemoTree.Get() : BuildDemoTree();
    UDialogueRunner* Runner = DialogueSubsystem->GetDialogueRunner();
    UChronicleDialoguePresentationController* PresentationController = DialogueSubsystem->GetPresentationController();
    PresentationController->BindRunner(Runner);
    EnsureDemoWidget();
    PresentationController->StartDialogue(DemoTree, EntryNode);
}

UChronicleDialogueWidget* AChronicleDialogueDemoActor::EnsureDemoWidget()
{
    if (!bCreateDefaultWidget)
    {
        return RuntimeWidget;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return RuntimeWidget;
    }

    UChronicleDialogueSubsystem* DialogueSubsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
    if (!DialogueSubsystem)
    {
        return RuntimeWidget;
    }

    UChronicleDialoguePresentationController* PresentationController = DialogueSubsystem->GetPresentationController();
    if (!RuntimeWidget)
    {
        TSubclassOf<UChronicleDialogueWidget> WidgetClassToUse = DialogueWidgetClass;
        if (!WidgetClassToUse)
        {
            WidgetClassToUse = UChronicleDialogueDefaultWidget::StaticClass();
        }

        RuntimeWidget = CreateWidget<UChronicleDialogueWidget>(GetWorld(), WidgetClassToUse);
        if (RuntimeWidget)
        {
            RuntimeWidget->AddToViewport(WidgetZOrder);
        }
    }

    if (RuntimeWidget)
    {
        RuntimeWidget->BindPresentationController(PresentationController);
    }

    return RuntimeWidget;
}
