#include "UI/MissionEntryOnHUDWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"


const FLinearColor UMissionEntryOnHUDWidget::CompleteColor = FLinearColor(0.0f, 1.0f, 0.64f, 1.0f);
const FLinearColor UMissionEntryOnHUDWidget::IncompleteColor = FLinearColor::White;

void UMissionEntryOnHUDWidget::ChangeImage(UTexture2D* Image)
{
	if (MissionEntryImage) 
	{ 
		MissionEntryImage->SetBrushFromTexture(Image); 
	}
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

void UMissionEntryOnHUDWidget::SetTitle(const FText& InTitle)
{
	if (Text_Title) { Text_Title->SetText(InTitle); }
}

void UMissionEntryOnHUDWidget::SetDescription(const FText& InDesc)
{
	if (Text_Description) { Text_Description->SetText(InDesc); }
}

void UMissionEntryOnHUDWidget::SetProgress(int32 Current, int32 Goal)
{
	if (Text_Title)
	{
		Text_Progress->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Goal)));
	}
	
}

void UMissionEntryOnHUDWidget::SetCompleted(bool bCompleted)
{
	UpdateMissionEntryColor(bCompleted);
}