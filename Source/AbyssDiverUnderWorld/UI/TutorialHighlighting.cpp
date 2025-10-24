#include "UI/TutorialHighlighting.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTutorialHighlighting::HighlightStart(const FHighlightingInfo& Info)
{
	MovingCanvas->SetVisibility(ESlateVisibility::Visible);
	if (UCanvasPanelSlot* MovingSlot = Cast<UCanvasPanelSlot>(MovingCanvas->Slot))
	{
		MovingSlot->SetAnchors(Info.Anchors);
		MovingSlot->SetPosition(Info.Offset);
		MovingCanvas->SetRenderScale(FVector2D(1.0f, 1.0f) * FMath::Clamp(Info.Multiply, 1.0f, Info.Multiply));
	}
	PlayAnimation(ShowHighlighting);

	if (HighlightArrow)
	{
		UpdateArrowTransform(Info);
		HighlightArrow->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (HighlightArrowFadeAnim)
		{
			PlayAnimation(HighlightArrowFadeAnim, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f, true);
		}
	}
}

void UTutorialHighlighting::HighlightEnd()
{
	MovingCanvas->SetVisibility(ESlateVisibility::Hidden);
	if (UCanvasPanelSlot* MovingSlot = Cast<UCanvasPanelSlot>(MovingCanvas->Slot))
	{
		MovingSlot->SetPosition(FVector2D(0.0f, 0.0f));
	}
	if (IsAnimationPlaying(ShowHighlighting))
	{
		StopAnimation(ShowHighlighting);
	}

	if (HighlightArrow)
	{
		HighlightArrow->SetVisibility(ESlateVisibility::Hidden);
		if (HighlightArrowFadeAnim && IsAnimationPlaying(HighlightArrowFadeAnim))
		{
			StopAnimation(HighlightArrowFadeAnim);
		}
	}
}

void UTutorialHighlighting::UpdateArrowTransform(const FHighlightingInfo& Info)
{
	UCanvasPanelSlot* ArrowSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(HighlightArrow);
	if (!ArrowSlot) return;

	if (Info.bUseManualArrowTransform)
	{
		ArrowSlot->SetAnchors(FAnchors(Info.ArrowAnchor.X, Info.ArrowAnchor.Y));
		ArrowSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ArrowSlot->SetPosition(FVector2D::ZeroVector);
		HighlightArrow->SetRenderTransformAngle(Info.ArrowAngle);
		return; 
	}

	const FVector2D HighlightCenterAnchor = (Info.Anchors.Minimum + Info.Anchors.Maximum) / 2.0f;

	float ArrowAngle = 0.0f;
	FVector2D ArrowAnchorPosition = FVector2D::ZeroVector;
	const float ArrowPadding = 0.02f;

	if (HighlightCenterAnchor.X < 0.5f)
	{
		ArrowAngle = 180.0f;
		ArrowAnchorPosition.X = Info.Anchors.Maximum.X + ArrowPadding;
		ArrowAnchorPosition.Y = HighlightCenterAnchor.Y;
	}
	else
	{
		ArrowAngle = 0.0f;
		ArrowAnchorPosition.X = Info.Anchors.Minimum.X - ArrowPadding;
		ArrowAnchorPosition.Y = HighlightCenterAnchor.Y;
	}

	ArrowSlot->SetAnchors(FAnchors(ArrowAnchorPosition.X, ArrowAnchorPosition.Y));
	ArrowSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ArrowSlot->SetPosition(FVector2D::ZeroVector);
	HighlightArrow->SetRenderTransformAngle(ArrowAngle);
}