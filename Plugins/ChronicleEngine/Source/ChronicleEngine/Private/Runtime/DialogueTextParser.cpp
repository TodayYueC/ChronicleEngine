#include "Runtime/DialogueTextParser.h"

namespace
{
bool ParseTag(const FString& TagText, FName& OutName, FString& OutValue)
{
    FString Left;
    FString Right;
    if (TagText.Split(TEXT("="), &Left, &Right))
    {
        OutName = FName(*Left.TrimStartAndEnd());
        OutValue = Right.TrimStartAndEnd();
        return !OutName.IsNone();
    }

    OutName = FName(*TagText.TrimStartAndEnd());
    OutValue = TEXT("true");
    return !OutName.IsNone();
}

bool IsMomentaryTag(FName TagName)
{
    static const TSet<FName> MomentaryTags =
    {
        FName(TEXT("wait")),
        FName(TEXT("portrait")),
        FName(TEXT("camera")),
        FName(TEXT("sfx")),
        FName(TEXT("event"))
    };

    return MomentaryTags.Contains(TagName);
}
}

void UDialogueTextParser::ParseInlineTags(const FText& SourceText, TArray<FLineSegment>& OutSegments)
{
    OutSegments.Reset();

    const FString Source = SourceText.ToString();
    TMap<FName, FString> ActiveTags;
    FString Buffer;

    auto FlushBuffer = [&]()
    {
        if (!Buffer.IsEmpty())
        {
            FLineSegment Segment;
            Segment.Text = FText::FromString(Buffer);
            Segment.Tags = ActiveTags;
            OutSegments.Add(Segment);
            Buffer.Reset();
        }
    };

    for (int32 Index = 0; Index < Source.Len(); ++Index)
    {
        if (Source[Index] == TEXT('['))
        {
            const int32 ClosingIndex = Source.Find(TEXT("]"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Index + 1);
            if (ClosingIndex != INDEX_NONE)
            {
                const FString TagText = Source.Mid(Index + 1, ClosingIndex - Index - 1);
                FName TagName;
                FString TagValue;
                if (ParseTag(TagText, TagName, TagValue))
                {
                    FlushBuffer();
                    if (IsMomentaryTag(TagName))
                    {
                        FLineSegment Segment;
                        Segment.Tags.Add(TagName, TagValue);
                        OutSegments.Add(Segment);
                    }
                    else
                    {
                        ActiveTags.Add(TagName, TagValue);
                    }
                    Index = ClosingIndex;
                    continue;
                }
            }
        }

        Buffer.AppendChar(Source[Index]);
    }

    FlushBuffer();

    if (OutSegments.Num() == 0 && Source.IsEmpty())
    {
        FLineSegment EmptySegment;
        EmptySegment.Text = FText::GetEmpty();
        OutSegments.Add(EmptySegment);
    }
}
