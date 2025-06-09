// Fill out your copyright notice in the Description page of Project Settings.


#include "NameWidget.h"

#include "Components/RichTextBlock.h"

void UNameWidget::SetNameText(const FString& NewName)
{
	NameTextBlock->SetText(FText::FromString(NewName));
}
