#include "UI/Flipbooks/FlipbookWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "PaperFlipbook.h"
#include "PaperSprite.h"
#include "Components/Image.h"

void UFlipbookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}

void UFlipbookWidget::PlayFlipbook(EFlipbookType PlayingFlipbook, bool bShouldLoop, float PlayTime)
{
	if (PlayingFlipbook == EFlipbookType::MAX)
	{
		LOGV(Error, TEXT("Not Valid Flipbook Type"));
		StopFlipbook();
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		StopFlipbook();
		return;
	}

	CurrentPaperFlipbbook = PaperFlipbooks[PlayingFlipbook];
	if (CurrentPaperFlipbbook.IsValid() == false)
	{
		LOGV(Error, TEXT("Not Valid PapaerFlipbook"));
		StopFlipbook();
		return;
	}

	CachedInterval = 1.0f / CurrentPaperFlipbbook->GetFramesPerSecond();

	CurrentFlipbookType = PlayingFlipbook;
	CurrentFrameIndex = 0;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	CachedPlayTime = PlayTime;
	World->GetTimerManager().SetTimer(FlipAnimTimerHandle, this, &UFlipbookWidget::PlayNextFrame, CachedInterval, bShouldLoop);
}

void UFlipbookWidget::StopFlipbook()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(FlipAnimTimerHandle);
	SetVisibility(ESlateVisibility::Hidden);
	CurrentFlipbookType = EFlipbookType::MAX;
	CurrentFrameIndex = 0;
	CachedInterval = 999.0f;
	ElapsedTime = 0.0f;
	CachedPlayTime = 0.0f;
}

void UFlipbookWidget::PlayNextFrame()
{
	if (CachedPlayTime > 0.0f && CachedPlayTime <= ElapsedTime)
	{
		StopFlipbook();
		return;
	}

	if (CurrentPaperFlipbbook.IsValid() == false || CurrentPaperFlipbbook->GetNumFrames() <= CurrentFrameIndex)
	{
		StopFlipbook();
		return;
	}

	TWeakObjectPtr<UPaperSprite> FrameSprite = CurrentPaperFlipbbook->GetSpriteAtFrame(CurrentFrameIndex);
	if (FrameSprite.IsValid() == false)
	{
		LOGV(Error, TEXT("Frame Sprite Is Not Valid. Index : %d"), CurrentFrameIndex);
		StopFlipbook();
		return;
	}

	UTexture2D* FrameTexture = FrameSprite->GetBakedTexture();
	if (FrameTexture == nullptr)
	{
		LOGV(Error, TEXT("FrameTexure is not valid, Index : %d"), CurrentFrameIndex);
		StopFlipbook();
		return;
	}

	if (IsValid(FrameImage) == false)
	{
		LOGV(Error, TEXT("FrameImage is not valid, Index : %d"), CurrentFrameIndex);
		StopFlipbook();
		return;
	}

	if (CachedPlayTime > 0)
	{
		ElapsedTime += CachedInterval;
	}

	FrameImage->SetBrushFromTexture(FrameTexture);
	CurrentFrameIndex++;
}

bool UFlipbookWidget::IsPlaying() const
{
	return CurrentFlipbookType != EFlipbookType::MAX;
}