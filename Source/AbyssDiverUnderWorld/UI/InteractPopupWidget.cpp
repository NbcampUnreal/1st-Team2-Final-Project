// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InteractPopupWidget.h"

void UInteractPopupWidget::ConfirmClicked()
{
	if (OnPopupConfirmed.IsBound())
	{
		OnPopupConfirmed.Execute();
	}
	RemoveFromParent();
}

void UInteractPopupWidget::CancelClicked()
{
	if (OnPopupCanceled.IsBound())
	{
		OnPopupCanceled.Execute();
	}
	RemoveFromParent();
}
