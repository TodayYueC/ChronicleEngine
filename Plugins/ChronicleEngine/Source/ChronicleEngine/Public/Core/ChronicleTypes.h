#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "ChronicleTypes.generated.h"

class UDialogueTree;

UENUM(BlueprintType)
enum class EChronicleVariableType : uint8
{
    None,
    Bool,
    Int32,
    Float,
    String,
    Name,
    GameplayTag,
    Vector,
    List
};

UENUM(BlueprintType)
enum class EChronicleVariableScope : uint8
{
    Global,
    Local,
    External
};

UENUM(BlueprintType)
enum class EDialogueNodeType : uint8
{
    Root,
    Speech,
    Choice,
    Condition,
    Event,
    Wait,
    Random,
    Jump,
    Sequence,
    SubDialogue,
    Camera,
    Animation
};

UENUM(BlueprintType)
enum class EDialogueRunnerState : uint8
{
    Idle,
    Running,
    WaitingForInput,
    WaitingForChoice,
    WaitingForEvent,
    Paused
};

UENUM(BlueprintType)
enum class EDialogueTriggerType : uint8
{
    Proximity,
    Interact,
    Auto,
    Event
};

UENUM(BlueprintType)
enum class ETextRevealMode : uint8
{
    Typewriter,
    Instant
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FVariableValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    EChronicleVariableType Type = EChronicleVariableType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    int32 IntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FString StringValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FName NameValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FGameplayTag TagValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FVector VectorValue = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    TArray<FString> ListValue;

    static FVariableValue MakeBool(bool InValue);
    static FVariableValue MakeInt(int32 InValue);
    static FVariableValue MakeFloat(float InValue);
    static FVariableValue MakeString(const FString& InValue);
    static FVariableValue MakeName(FName InValue);
    static FVariableValue MakeTag(FGameplayTag InValue);
    static FVariableValue MakeVector(const FVector& InValue);

    bool AsBool(bool bDefault = false) const;
    double AsNumber(double Default = 0.0) const;
    FString AsString() const;
    bool IsNumeric() const;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FVariableDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FGameplayTag VariableTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    EChronicleVariableScope Scope = EChronicleVariableScope::Global;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FVariableValue DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Variable")
    FText DisplayName;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FLineSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Text")
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Text")
    TMap<FName, FString> Tags;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    FGameplayTag SpeakerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    FGameplayTag EmotionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    FName VoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    TArray<FGameplayTag> MetaTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    float WaitTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Dialogue")
    TArray<FLineSegment> Segments;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Choice")
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Choice")
    FString VisibilityCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Choice")
    int32 TargetOutputIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    FGuid NodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    EDialogueNodeType NodeType = EDialogueNodeType::Speech;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    TArray<FDialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    TArray<FDialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    FString ConditionExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    TMap<FName, FString> EventPayload;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    bool bEventIsAsync = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    float WaitTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    int32 DefaultOutputIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    TSoftObjectPtr<UDialogueTree> TargetTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    FName TargetEntryNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    bool bReturnToNextNodeOnSubDialogueEnd = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Node")
    bool bAutoSelectIfSingle = false;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueEdge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    FGuid FromNodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    FGuid ToNodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    int32 FromSlotIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    int32 ToSlotIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    FString ConditionExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Edge")
    float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueNodeEditorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FGuid NodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    bool bBreakpointEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FString BreakpointNote;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FChronicleSoftLockMetadata
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    bool bLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FString OwnerUserName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FString OwnerMachineName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FGuid SessionGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FDateTime LockedAtUtc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Editor")
    FString Note;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueHistoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FGuid TreeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FGuid NodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FGameplayTag SpeakerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|History")
    FText Text;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueEventData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Event")
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Event")
    TMap<FName, FString> Payload;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Event")
    bool bAsync = false;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    FGuid CurrentTreeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    FGuid CurrentNodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    int32 CurrentLineIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    EDialogueRunnerState RunnerState = EDialogueRunnerState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    TMap<FGameplayTag, FVariableValue> GlobalVariables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    TMap<FGameplayTag, FVariableValue> LocalVariables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    TArray<FDialogueHistoryEntry> History;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Save")
    TArray<FString> SeenDialogueHashes;
};
