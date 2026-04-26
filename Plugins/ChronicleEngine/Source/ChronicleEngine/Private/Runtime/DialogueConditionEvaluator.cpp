#include "Runtime/DialogueConditionEvaluator.h"

#include "Core/ChronicleTypes.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
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

enum class EExpressionNodeType
{
    Literal,
    Variable,
    Not,
    And,
    Or,
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual
};

struct FExpressionNode
{
    EExpressionNodeType Type = EExpressionNodeType::Literal;
    FVariableValue Literal;
    FName VariableName;
    TSharedPtr<FExpressionNode> Left;
    TSharedPtr<FExpressionNode> Right;

    FVariableValue Evaluate(const UVariableBank* VariableBank, bool& bSuccess) const
    {
        switch (Type)
        {
        case EExpressionNodeType::Literal:
            return Literal;

        case EExpressionNodeType::Variable:
        {
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

        case EExpressionNodeType::Not:
            return FVariableValue::MakeBool(!EvaluateChild(Left, VariableBank, bSuccess).AsBool());

        case EExpressionNodeType::And:
        {
            const FVariableValue LeftValue = EvaluateChild(Left, VariableBank, bSuccess);
            if (!bSuccess || !LeftValue.AsBool())
            {
                return FVariableValue::MakeBool(false);
            }
            return FVariableValue::MakeBool(EvaluateChild(Right, VariableBank, bSuccess).AsBool());
        }

        case EExpressionNodeType::Or:
        {
            const FVariableValue LeftValue = EvaluateChild(Left, VariableBank, bSuccess);
            if (!bSuccess || LeftValue.AsBool())
            {
                return FVariableValue::MakeBool(LeftValue.AsBool());
            }
            return FVariableValue::MakeBool(EvaluateChild(Right, VariableBank, bSuccess).AsBool());
        }

        case EExpressionNodeType::Equal:
        case EExpressionNodeType::NotEqual:
        case EExpressionNodeType::Greater:
        case EExpressionNodeType::GreaterEqual:
        case EExpressionNodeType::Less:
        case EExpressionNodeType::LessEqual:
            return EvaluateBinaryComparison(VariableBank, bSuccess);
        }

        bSuccess = false;
        return FVariableValue();
    }

private:
    static FVariableValue EvaluateChild(const TSharedPtr<FExpressionNode>& Child, const UVariableBank* VariableBank, bool& bSuccess)
    {
        if (!Child.IsValid())
        {
            bSuccess = false;
            return FVariableValue();
        }

        return Child->Evaluate(VariableBank, bSuccess);
    }

    FVariableValue EvaluateBinaryComparison(const UVariableBank* VariableBank, bool& bSuccess) const
    {
        const FVariableValue LeftValue = EvaluateChild(Left, VariableBank, bSuccess);
        if (!bSuccess)
        {
            return FVariableValue::MakeBool(false);
        }

        const FVariableValue RightValue = EvaluateChild(Right, VariableBank, bSuccess);
        if (!bSuccess)
        {
            return FVariableValue::MakeBool(false);
        }

        if (Type == EExpressionNodeType::Equal || Type == EExpressionNodeType::NotEqual)
        {
            const bool bEqual = ValuesEqual(LeftValue, RightValue);
            return FVariableValue::MakeBool(Type == EExpressionNodeType::Equal ? bEqual : !bEqual);
        }

        if (!LeftValue.IsNumeric() || !RightValue.IsNumeric())
        {
            bSuccess = false;
            return FVariableValue::MakeBool(false);
        }

        const double Lhs = LeftValue.AsNumber();
        const double Rhs = RightValue.AsNumber();
        bool bResult = false;
        switch (Type)
        {
        case EExpressionNodeType::Greater:
            bResult = Lhs > Rhs;
            break;
        case EExpressionNodeType::GreaterEqual:
            bResult = Lhs >= Rhs;
            break;
        case EExpressionNodeType::Less:
            bResult = Lhs < Rhs;
            break;
        case EExpressionNodeType::LessEqual:
            bResult = Lhs <= Rhs;
            break;
        default:
            break;
        }

        return FVariableValue::MakeBool(bResult);
    }

    static bool ValuesEqual(const FVariableValue& LeftValue, const FVariableValue& RightValue)
    {
        if (LeftValue.IsNumeric() && RightValue.IsNumeric())
        {
            return FMath::IsNearlyEqual(LeftValue.AsNumber(), RightValue.AsNumber());
        }

        return LeftValue.AsString().Equals(RightValue.AsString(), ESearchCase::CaseSensitive);
    }
};

class FParser
{
public:
    explicit FParser(const FString& InExpression)
        : Lexer(InExpression)
    {
        Current = Lexer.Next();
    }

    TSharedPtr<FExpressionNode> Parse(bool& bOutSuccess)
    {
        bSuccess = true;
        TSharedPtr<FExpressionNode> Result = ParseOr();
        bOutSuccess = bSuccess && Result.IsValid() && Current.Type == ETokenType::End;
        return bOutSuccess ? Result : nullptr;
    }

private:
    TSharedPtr<FExpressionNode> ParseOr()
    {
        TSharedPtr<FExpressionNode> Left = ParseAnd();
        while (Current.Type == ETokenType::Or)
        {
            Consume();
            Left = MakeBinary(EExpressionNodeType::Or, Left, ParseAnd());
        }
        return Left;
    }

    TSharedPtr<FExpressionNode> ParseAnd()
    {
        TSharedPtr<FExpressionNode> Left = ParseEquality();
        while (Current.Type == ETokenType::And)
        {
            Consume();
            Left = MakeBinary(EExpressionNodeType::And, Left, ParseEquality());
        }
        return Left;
    }

    TSharedPtr<FExpressionNode> ParseEquality()
    {
        TSharedPtr<FExpressionNode> Left = ParseComparison();
        while (Current.Type == ETokenType::Equal || Current.Type == ETokenType::NotEqual)
        {
            const EExpressionNodeType Operator = Current.Type == ETokenType::Equal
                ? EExpressionNodeType::Equal
                : EExpressionNodeType::NotEqual;
            Consume();
            Left = MakeBinary(Operator, Left, ParseComparison());
        }
        return Left;
    }

    TSharedPtr<FExpressionNode> ParseComparison()
    {
        TSharedPtr<FExpressionNode> Left = ParseUnary();
        while (Current.Type == ETokenType::Greater || Current.Type == ETokenType::GreaterEqual
            || Current.Type == ETokenType::Less || Current.Type == ETokenType::LessEqual)
        {
            const EExpressionNodeType Operator = TokenToComparisonNodeType(Current.Type);
            Consume();
            Left = MakeBinary(Operator, Left, ParseUnary());
        }
        return Left;
    }

    TSharedPtr<FExpressionNode> ParseUnary()
    {
        if (Current.Type == ETokenType::Not)
        {
            Consume();
            return MakeUnary(EExpressionNodeType::Not, ParseUnary());
        }

        return ParsePrimary();
    }

    TSharedPtr<FExpressionNode> ParsePrimary()
    {
        switch (Current.Type)
        {
        case ETokenType::True:
            Consume();
            return MakeLiteral(FVariableValue::MakeBool(true));

        case ETokenType::False:
            Consume();
            return MakeLiteral(FVariableValue::MakeBool(false));

        case ETokenType::Number:
        {
            const FString NumberText = Current.Text;
            Consume();
            return MakeLiteral(NumberText.Contains(TEXT("."))
                ? FVariableValue::MakeFloat(FCString::Atof(*NumberText))
                : FVariableValue::MakeInt(FCString::Atoi(*NumberText)));
        }

        case ETokenType::String:
        {
            const FString Text = Current.Text;
            Consume();
            return MakeLiteral(FVariableValue::MakeString(Text));
        }

        case ETokenType::Identifier:
        {
            const FName VariableName(*Current.Text);
            Consume();
            TSharedPtr<FExpressionNode> Node = MakeShared<FExpressionNode>();
            Node->Type = EExpressionNodeType::Variable;
            Node->VariableName = VariableName;
            return Node;
        }

        case ETokenType::LeftParen:
        {
            Consume();
            TSharedPtr<FExpressionNode> Node = ParseOr();
            if (Current.Type != ETokenType::RightParen)
            {
                bSuccess = false;
                return nullptr;
            }
            Consume();
            return Node;
        }

        default:
            bSuccess = false;
            return nullptr;
        }
    }

    TSharedPtr<FExpressionNode> MakeLiteral(const FVariableValue& Value) const
    {
        TSharedPtr<FExpressionNode> Node = MakeShared<FExpressionNode>();
        Node->Type = EExpressionNodeType::Literal;
        Node->Literal = Value;
        return Node;
    }

    TSharedPtr<FExpressionNode> MakeUnary(EExpressionNodeType Type, TSharedPtr<FExpressionNode> Child)
    {
        if (!Child.IsValid())
        {
            bSuccess = false;
            return nullptr;
        }

        TSharedPtr<FExpressionNode> Node = MakeShared<FExpressionNode>();
        Node->Type = Type;
        Node->Left = Child;
        return Node;
    }

    TSharedPtr<FExpressionNode> MakeBinary(EExpressionNodeType Type, TSharedPtr<FExpressionNode> Left, TSharedPtr<FExpressionNode> Right)
    {
        if (!Left.IsValid() || !Right.IsValid())
        {
            bSuccess = false;
            return nullptr;
        }

        TSharedPtr<FExpressionNode> Node = MakeShared<FExpressionNode>();
        Node->Type = Type;
        Node->Left = Left;
        Node->Right = Right;
        return Node;
    }

    static EExpressionNodeType TokenToComparisonNodeType(ETokenType TokenType)
    {
        switch (TokenType)
        {
        case ETokenType::Greater:
            return EExpressionNodeType::Greater;
        case ETokenType::GreaterEqual:
            return EExpressionNodeType::GreaterEqual;
        case ETokenType::Less:
            return EExpressionNodeType::Less;
        case ETokenType::LessEqual:
            return EExpressionNodeType::LessEqual;
        default:
            return EExpressionNodeType::Equal;
        }
    }

    void Consume()
    {
        Current = Lexer.Next();
    }

    FLexer Lexer;
    FToken Current;
    bool bSuccess = true;
};

struct FCompiledExpression
{
    TSharedPtr<FExpressionNode> Root;
    bool bCompileSuccess = false;
};

FCriticalSection ExpressionCacheCriticalSection;
TMap<FString, FCompiledExpression> ExpressionCache;

FCompiledExpression GetOrCompileExpression(const FString& TrimmedExpression)
{
    FScopeLock Lock(&ExpressionCacheCriticalSection);

    if (const FCompiledExpression* CachedExpression = ExpressionCache.Find(TrimmedExpression))
    {
        return *CachedExpression;
    }

    bool bCompileSuccess = false;
    FParser Parser(TrimmedExpression);
    FCompiledExpression CompiledExpression;
    CompiledExpression.Root = Parser.Parse(bCompileSuccess);
    CompiledExpression.bCompileSuccess = bCompileSuccess && CompiledExpression.Root.IsValid();

    constexpr int32 MaxCachedExpressions = 256;
    if (ExpressionCache.Num() >= MaxCachedExpressions)
    {
        ExpressionCache.Reset();
    }

    ExpressionCache.Add(TrimmedExpression, CompiledExpression);
    return CompiledExpression;
}
}

bool UDialogueConditionEvaluator::EvaluateCondition(const FString& Expression, const UVariableBank* VariableBank, bool& bSuccess)
{
    const FString Trimmed = Expression.TrimStartAndEnd();
    if (Trimmed.IsEmpty())
    {
        bSuccess = true;
        return true;
    }

    if (Trimmed.Equals(TEXT("true"), ESearchCase::IgnoreCase))
    {
        bSuccess = true;
        return true;
    }

    if (Trimmed.Equals(TEXT("false"), ESearchCase::IgnoreCase))
    {
        bSuccess = true;
        return false;
    }

    const ChronicleCondition::FCompiledExpression CompiledExpression = ChronicleCondition::GetOrCompileExpression(Trimmed);
    if (!CompiledExpression.bCompileSuccess || !CompiledExpression.Root.IsValid())
    {
        bSuccess = false;
        return false;
    }

    bSuccess = true;
    const FVariableValue Result = CompiledExpression.Root->Evaluate(VariableBank, bSuccess);
    return bSuccess && Result.AsBool();
}
