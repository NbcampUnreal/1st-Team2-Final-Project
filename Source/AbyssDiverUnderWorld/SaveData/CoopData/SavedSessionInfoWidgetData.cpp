#include "SaveData/CoopData/SavedSessionInfoWidgetData.h"

#include "UI/SaveLoads/SavedSessionInfoWidget.h"

void USavedSessionInfoWidgetData::InitData(const FString& InSavedSessionName, const FString& InSavedSessionDateText, int32 InInfoIndex)
{
	SavedSessionName = InSavedSessionName;
	SavedSessionDateText = InSavedSessionDateText;
	InfoIndex = InInfoIndex;
}
