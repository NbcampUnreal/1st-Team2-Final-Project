#include "UI/MissionEntryOnHUDWidget.h"

#include "Components/Image.h"

const FLinearColor UMissionEntryOnHUDWidget::CompleteColor = FLinearColor(0.0f, 1.0f, 0.64f, 1.0f);
const FLinearColor UMissionEntryOnHUDWidget::IncompleteColor = FLinearColor::White;

void UMissionEntryOnHUDWidget::ChangeImage(UTexture2D* Image)
{
	MissionEntryImage->SetBrushFromTexture(Image);
}

void UMissionEntryOnHUDWidget::UpdateMissionEntryColor(bool bIsMissionCompleted)
{
	if (bIsMissionCompleted)
	{
		MissionEntryImage->SetColorAndOpacity(CompleteColor);
	}
	else
	{
		MissionEntryImage->SetColorAndOpacity(IncompleteColor);
	}
}

void UMissionEntryOnHUDWidget::SetVisible(bool bShouldVisible)
{
	if (bShouldVisible)
	{
		MissionEntryImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		MissionEntryImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
