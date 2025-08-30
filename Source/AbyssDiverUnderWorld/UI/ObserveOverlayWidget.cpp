#include "UI/ObserveOverlayWidget.h"

//Components
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ObservedTargetWidget.h"

void UObserveOverlayWidget::SetObserveModeActive(bool bActive)
{
	SetVisibility(bActive ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (bActive) BP_OnObserveModeActivated(); else BP_OnObserveModeDeactivated();
}

void UObserveOverlayWidget::SetRingVisible(bool bVisible)
{
	BP_OnSetRingVisible(bVisible);
}

void UObserveOverlayWidget::SetRingProgress(float Progress01)
{
	BP_OnSetRingProgress(Progress01);
}

void UObserveOverlayWidget::SetRingScreenPos(const FVector2D& ScreenPos)
{
	BP_OnSetRingScreenPosition(ScreenPos);
}

void UObserveOverlayWidget::PlayAcquirePulse()
{
	BP_OnAcquirePulse();
}

void UObserveOverlayWidget::AttachObservedTargetWidget(UObservedTargetWidget* InWidget)
{
	BP_AttachObservedTarget(InWidget);
}
