#include "UI/ObservedTargetWidget.h"

//Component
#include "Components/TextBlock.h"

void UObservedTargetWidget::SetTargetName(const FText& NewName)
{
	if (NewName.EqualTo(CachedName))
		return;

	CachedName = NewName;

	BP_OnNameChanged(NewName);

	if (Text_TargetName)
	{
		Text_TargetName->SetText(NewName);
	}
}

void UObservedTargetWidget::SetObservedTargetVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	if (bVisible) 
		BP_OnShow();
	else
		BP_OnHide();
}
