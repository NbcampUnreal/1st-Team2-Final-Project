#include "UI/WaitForSomethingWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"

void UWaitForSomethingWidget::SetWaitText(const FString& NewText)
{
	if (IsValid(WaitText) == false)
	{
		LOGV(Error, TEXT(" WaitText is not valid"));
		return;
	}

	WaitText->SetText(FText::FromString(NewText));
}
