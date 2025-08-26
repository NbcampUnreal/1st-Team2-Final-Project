// Fill out your copyright notice in the Description page of Project Settings.


#include "ADNameWidget.h"

#include "Components/RichTextBlock.h"

void UADNameWidget::SetNameText(const FString& NewName)
{
	NameTextBlock->SetText(FText::FromString(NewName));
}
