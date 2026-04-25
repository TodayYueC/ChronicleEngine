#include "Core/ChronicleTypes.h"

FVariableValue FVariableValue::MakeBool(bool InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::Bool;
    Value.BoolValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeInt(int32 InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::Int32;
    Value.IntValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeFloat(float InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::Float;
    Value.FloatValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeString(const FString& InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::String;
    Value.StringValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeName(FName InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::Name;
    Value.NameValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeTag(FGameplayTag InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::GameplayTag;
    Value.TagValue = InValue;
    return Value;
}

FVariableValue FVariableValue::MakeVector(const FVector& InValue)
{
    FVariableValue Value;
    Value.Type = EChronicleVariableType::Vector;
    Value.VectorValue = InValue;
    return Value;
}

bool FVariableValue::AsBool(bool bDefault) const
{
    switch (Type)
    {
    case EChronicleVariableType::Bool:
        return BoolValue;
    case EChronicleVariableType::Int32:
        return IntValue != 0;
    case EChronicleVariableType::Float:
        return !FMath::IsNearlyZero(FloatValue);
    case EChronicleVariableType::String:
        return !StringValue.IsEmpty() && !StringValue.Equals(TEXT("false"), ESearchCase::IgnoreCase);
    case EChronicleVariableType::Name:
        return !NameValue.IsNone();
    case EChronicleVariableType::GameplayTag:
        return TagValue.IsValid();
    case EChronicleVariableType::Vector:
        return !VectorValue.IsNearlyZero();
    case EChronicleVariableType::List:
        return ListValue.Num() > 0;
    default:
        return bDefault;
    }
}

double FVariableValue::AsNumber(double Default) const
{
    switch (Type)
    {
    case EChronicleVariableType::Bool:
        return BoolValue ? 1.0 : 0.0;
    case EChronicleVariableType::Int32:
        return static_cast<double>(IntValue);
    case EChronicleVariableType::Float:
        return static_cast<double>(FloatValue);
    case EChronicleVariableType::String:
    {
        if (StringValue.IsNumeric())
        {
            return FCString::Atod(*StringValue);
        }
        return Default;
    }
    default:
        return Default;
    }
}

FString FVariableValue::AsString() const
{
    switch (Type)
    {
    case EChronicleVariableType::Bool:
        return BoolValue ? TEXT("true") : TEXT("false");
    case EChronicleVariableType::Int32:
        return FString::FromInt(IntValue);
    case EChronicleVariableType::Float:
        return FString::SanitizeFloat(FloatValue);
    case EChronicleVariableType::String:
        return StringValue;
    case EChronicleVariableType::Name:
        return NameValue.ToString();
    case EChronicleVariableType::GameplayTag:
        return TagValue.ToString();
    case EChronicleVariableType::Vector:
        return VectorValue.ToString();
    case EChronicleVariableType::List:
        return FString::Join(ListValue, TEXT(","));
    default:
        return FString();
    }
}

bool FVariableValue::IsNumeric() const
{
    return Type == EChronicleVariableType::Bool
        || Type == EChronicleVariableType::Int32
        || Type == EChronicleVariableType::Float
        || (Type == EChronicleVariableType::String && StringValue.IsNumeric());
}

