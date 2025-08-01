#include "UI/SaveLoads/SavedSessionInfoWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "SaveData/CoopData/SavedSessionInfoWidgetData.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

const FLinearColor USavedSessionInfoWidget::ClickedColor = FLinearColor::Blue;
const FLinearColor USavedSessionInfoWidget::NotClickedColor = FLinearColor(0, 0, 0, 0);

void USavedSessionInfoWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	USavedSessionInfoWidgetData* WidgetData = Cast<USavedSessionInfoWidgetData>(ListItemObject);
	if (WidgetData == nullptr)
	{
		LOGV(Error, TEXT("Not Vaild Wiget Data"));
		return;
	}

	SetSavedSessionNameText(WidgetData->SavedSessionName);
	SetSavedSessionDateText(WidgetData->SavedSessionDateText);
	WidgetIndex = WidgetData->InfoIndex;

	WidgetData->OnInfoWidgetUpdated.ExecuteIfBound(this);
}

FReply USavedSessionInfoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnInfoWidgetClickedDelegate.Broadcast(WidgetIndex);
	LOGV(Log, TEXT("Clicked Index : %d"), WidgetIndex);

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USavedSessionInfoWidget::SetBackgroundColor(const FLinearColor& NewColor)
{
	if (IsValid(BackgroundImage) == false)
	{
		LOGV(Error, TEXT("BackgroundImage is not valid"));
		return;
	}

	BackgroundImage->SetColorAndOpacity(NewColor);
}

void USavedSessionInfoWidget::SetSavedSessionNameText(const FString& NewText)
{
	if (IsValid(SavedSessionNameText) == false)
	{
		LOGV(Error, TEXT("SavedSessionNameText is not valid"));
		return;
	}

	SavedSessionNameText->SetText(FText::FromString(NewText));
}

void USavedSessionInfoWidget::SetSavedSessionDateText(const FString& NewText)
{
	if (IsValid(SavedSessionDateText) == false)
	{
		LOGV(Error, TEXT("SavedSessionDataText is not valid"));
		return;
	}

	SavedSessionDateText->SetText(FText::FromString(NewText));
}

void USavedSessionInfoWidget::SetToClickedColor()
{
	SetBackgroundColor(ClickedColor);
}

void USavedSessionInfoWidget::SetToNotClickedColor()
{
	SetBackgroundColor(NotClickedColor);
}

int32 USavedSessionInfoWidget::GetWidgetIndex() const
{
	return WidgetIndex;
}
