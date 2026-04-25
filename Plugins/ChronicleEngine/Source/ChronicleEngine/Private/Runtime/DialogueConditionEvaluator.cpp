#include "Runtime/DialogueConditionEvaluator.h"

#include "Core/ChronicleTypes.h"
#include "Runtime/VariableBank.h"

namespace ChronicleCondition
{
enum class ETokenType
{
    End,
    Identifier,
    Number,
    String,
    True,
    False,
    And,
    Or,
    Not,
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    LeftParen,
    RightParen
};

struct FToken
{
    ETokenType Type = ETokenType::End;
    FString Text;
};

class FLexer
{
public:
    explicit FLexer(const FString& InExpression)
        : Expression(InExpression)
    {
    }

    FToken Next()
    {
        SkipWhitespace();

        if (Index >= Expression.Len())
        {
            return { ETokenType::End, FString() };
        }

        const TCHAR Char = Expression[Index];

        if (FChar::IsAlpha(Char) || Char == TEXT('_'))
        {
            const int32 Start = Index;
            while (Index < Expression.Len())
            {
                const TCHAR Current = Expression[Index];
                if (!FChar::IsAlnum(Current) && Current != TEXT('_') && Current != TEXT('.'))
                {
                    break;
                }
                ++Index;
            }

            const FString Text = Expression.Mid(Start, Index - Start);
            if (Text.Equals(TEXT("AND"), ESearchCase::IgnoreCase))
            {
                return { ETokenType::And, Text };
            }
            if (Text.Equals(TEXT("OR"), ESearchCase::IgnoreCase))
            {
                return { ETokenType::Or, Text };
            }
            if (Text.Equals(TEXT("NOT"), ESearchCase::IgnoreCase))
            {
                return { ETokenType::Not, Text };
            }
            if (Text.Equals(TEXT("true"), ESearchCase::IgnoreCase))
            {
                return { ETokenType::True, Text };
            }
            if (Text.Equals(TEXT("false"), ESearchCase::IgnoreCase))
            {
                return { ETokenType::False, Text };
            }

            return { ETokenType::Identifier, Text };
        }

        if (FChar::IsDigit(Char) || (Char == TEXT('-') && Index + 1 < Expression.Len() && FChar::IsDigit(Expression[Index + 1])))
        {
            const int32 Start = Index++;
            while (Index < Expression.Len() && (FChar::IsDigit(Expression[Index]) || Expression[Index] == TEXT('.')))
            {
                ++Index;
            }

            return { ETokenType::Number, Expression.Mid(Start, Index - Start) };
        }

        if (Char == TEXT('"') || Char == TEXT('\''))
        {
            const TCHAR Quote = Char;
            ++Index;
            FString Text;
            while (Index < Expression.Len() && Expression[Index] != Quote)
            {
                Text.AppendChar(Expression[Index++]);
            }
            if (Index < Expression.Len() && Expression[Index] == Quote)
            {
                ++Index;
            }
            return { ETokenType::String, Text };
        }

        ++Index;
        switch (Char)
        {
        case TEXT('&'):
            if (ConsumeIf(TEXT('&')))
            {
                return { ETokenType::And, TEXT("&&") };
            }
            break;
        case TEXT('|'):
            if (ConsumeIf(TEXT('|')))
            {
                return { ETokenType::Or, TEXT("||") };
            }
            break;
        case TEXT('!'):
            if (ConsumeIf(TEXT('=')))
            {
                return { ETokenType::NotEqual, TEXT("!=") };
            }
            return { ETokenType::Not, TEXT("!") };
        case TEXT('='):
            if (ConsumeIf(TEXT('=')))
            {
                return { ETokenType::Equal, TEXT("==") };
            }
            break;
        case TEXT('>'):
            if (ConsumeIf(TEXT('=')))
            {
                return { ETokenType::GreaterEqual, TEXT(">=") };
            }
            return { ETokenType::Greater, TEXT(">") };
        case TEXT('<'):
            if (ConsumeIf(TEXT('=')))
            {
                return { ETokenType::LessEqual, TEXT("<=") };
            }
            return { ETokenType::Less, TEXT("<") };
        case TEXT('('):
            return { ETokenType::LeftParen, TEXT("(") };
        case TEXT(')'):
            return { ETokenType::RightParen, TEXT(")") };
        default:
            break;
        }

        return { ETokenType::End, FString() };
    }

private:
    void SkipWhitespace()
    {
        while (Index < Expression.Len() && FChar::IsWhitespace(Expression[Index]))
        {
            ++Index;
        }
    }

    bool ConsumeIf(TCHAR Expected)
    {
        if (Index < Expression.Len() && Expression[Index] == Expected)
        {
            ++Index;
            return true;
        }
        return false;
    }

    FString Expression;
    int32 Index = 0;
};

class FParser
{
public:
    FParser(const FString& InExpression, const UVariableBank* InVariableBank)
        : Lexer(InExpression)
        , VariableBank(InVariableBank)
    {
        Current = Lexer.Next();
    }

    FVariableValue Parse(bool& bOutSuccess)
    {
        bSuccess = true;
        FVariableValue Result = ParseOr();
        bOutSuccess = bSuccess && Current.Type == ETokenType::End;
        return Result;
    }

private:
    FVariableValue ParseOr()
    {
        FVariableValue Left = ParseAnd();
        while (Current.Type == ETokenType::Or)
        {
            Consume();
            const FVariableValue Right = ParseAnd();
            Left = FVariableValue::MakeBool(Left.AsBool() || Right.AsBool());
        }
        return Left;
    }

    FVariableValue ParseAnd()
    {
        FVariableValue Left = ParseEquality();
        while (Current.Type == ETokenType::And)
        {
            Consume();
            const FVariableValue Right = ParseEquality();
            Left = FVariableValue::MakeBool(Left.AsBool() && Right.AsBool());
        }
        return Left;
    }

    FVariableValue ParseEquality()
    {
        FVariableValue Left = ParseComparison();
        while (Current.Type == ETokenType::Equal || Current.Type == ETokenType::NotEqual)
        {
            const ETokenType Operator = Current.Type;
            Consume();
            const FVariableValue Right = ParseComparison();
            const bool bEqual = ValuesEqual(Left, Right);
            Left = FVariableValue::MakeBool(Operator == ETokenType::Equal ? bEqual : !bEqual);
        }
        return Left;
    }

    FVariableValue ParseComparison()
    {
        FVariableValue Left = ParseUnary();
        while (Current.Type == ETokenType::Greater || Current.Type == ETokenType::GreaterEqual
            || Current.Type == ETokenType::Less || Current.Type == ETokenType::LessEqual)
        {
            const ETokenType Operator = Current.Type;
            Consume();
            const FVariableValue Right = ParseUnary();

            if (!Left.IsNumeric() || !Right.IsNumeric())
            {
                bSuccess = false;
                return FVariableValue::MakeBool(false);
            }

            const double Lhs = Left.AsNumber();
            const double Rhs = Right.AsNumber();
            bool bResult = false;
            switch (Operator)
            {
            case ETokenType::Greater:
                bResult = Lhs > Rhs;
                break;
            case ETokenType::GreaterEqual:
                bResult = Lhs >= Rhs;
                break;
            case ETokenType::Less:
                bResult = Lhs < Rhs;
                break;
            case ETokenType::LessEqual:
                bResult = Lhs <= Rhs;
                break;
            default:
                break;
            }
            Left = FVariableValue::MakeBool(bResult);
        }
        return Left;
    }

    FVariableValue ParseUnary()
    {
        if (Current.Type == ETokenType::Not)
        {
            Consume();
            return FVariableValue::MakeBool(!ParseUnary().AsBool());
        }

        return ParsePrimary();
    }

    FVariableValue ParsePrimary()
    {
        switch (Current.Type)
        {
        case ETokenType::True:
            Consume();
            return FVariableValue::MakeBool(true);
        case ETokenType::False:
            Consume();
            return FVariableValue::MakeBool(false);
        case ETokenType::Number:
        {
            const FString NumberText = Current.Text;
            Consume();
            return NumberText.Contains(TEXT(".")) ? FVariableValue::MakeFloat(FCString::Atof(*NumberText)) : FVariableValue::MakeInt(FCString::Atoi(*NumberText));
        }
        case ETokenType::String:
        {
            const FString Text = Current.Text;
            Consume();
            return FVariableValue::MakeString(Text);
        }
        case ETokenType::Identifier:
        {
            const FName VariableName(*Current.Text);
            Consume();
            if (!VariableBank)
            {
                bSuccess = false;
                return FVariableValue();
            }

            FVariableValue Value;
            if (!VariableBank->GetVariableByName(VariableName, Value))
            {
                bSuccess = false;
                return FVariableValue();
            }
            return Value;
        }
        case ETokenType::LeftParen:
        {
            Consume();
            FVariableValue Value = ParseOr();
            if (Current.Type != ETokenType::RightParen)
            {
                bSuccess = false;
                return FVariableValue();
            }
            Consume();
            return Value;
        }
        default:
            bSuccess = false;
            return FVariableValue();
        }
    }

    bool ValuesEqual(const FVariableValue& Left, const FVariableValue& Right) const
    {
        if (Left.IsNumeric() && Right.IsNumeric())
        {
            return FMath::IsNearlyEqual(Left.AsNumber(), Right.AsNumber());
        }

        return Left.AsString().Equals(Right.AsString(), ESearchCase::CaseSensitive);
    }

    void Consume()
    {
        Current = Lexer.Next();
    }

    FLexer Lexer;
    FToken Current;
    const UVariableBank* VariableBank = nullptr;
    bool bSuccess = true;
};
}

bool UDialogueConditionEvaluator::EvaluateCondition(const FString& Expression, const UVariableBank* VariableBank, bool& bSuccess)
{
    const FString Trimmed = Expression.TrimStartAndEnd();
    if (Trimmed.IsEmpty())
    {
        bSuccess = true;
        return true;
    }

    ChronicleCondition::FParser Parser(Trimmed, VariableBank);
    const FVariableValue Result = Parser.Parse(bSuccess);
    return bSuccess && Result.AsBool();
}

