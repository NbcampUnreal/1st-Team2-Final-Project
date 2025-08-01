#include "UI/ConfirmationDialogWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UConfirmationDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (YesButton->OnClicked.IsAlreadyBound(this, &UConfirmationDialogWidget::OnYesButtonClicked))
	{
		return;
	}

	YesButton->OnClicked.AddDynamic(this, &UConfirmationDialogWidget::OnYesButtonClicked);
	NoButton->OnClicked.AddDynamic(this, &UConfirmationDialogWidget::OnNoButtonClicked);

}

void UConfirmationDialogWidget::SetConfirmationText(const FString& NewText)
{
	if (IsValid(ConfirmationText) == false)
	{
		LOGV(Error, TEXT("ConfirmationText is not valid"));
		return;
	}

	ConfirmationText->SetText(FText::FromString(NewText));
}

void UConfirmationDialogWidget::OnYesButtonClicked()
{
	OnYesButtonClickedDelegate.Broadcast();
}

void UConfirmationDialogWidget::OnNoButtonClicked()
{
	OnNoButtonClickedDelegate.Broadcast();
}
