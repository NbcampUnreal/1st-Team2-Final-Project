#include "UI/SelectedMissionSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USelectedMissionSlot::OnMissionFinished()
{
	Text_Title->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	MissionBG->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	FinishLineImage->SetVisibility(ESlateVisibility::Visible);
}

void USelectedMissionSlot::SetMissionTitle(const FString& Title)
{
	Text_Title->SetText(FText::FromString(Title));
}
