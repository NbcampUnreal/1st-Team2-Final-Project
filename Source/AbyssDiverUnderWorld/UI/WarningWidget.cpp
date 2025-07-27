// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WarningWidget.h"

void UWarningWidget::SetbShowWarning(bool newbShowWarning)
{
	bShowWarning = newbShowWarning;

	newbShowWarning ? ShowWarning() : HideWarning();
}