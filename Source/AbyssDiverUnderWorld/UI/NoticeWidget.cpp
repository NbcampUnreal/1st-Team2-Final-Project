// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeWidget.h"
#include "Components/TextBlock.h"

void UNoticeWidget::SetNoticeText(const FString& NewNotice)
{
	NoticeText->SetText(FText::FromString(NewNotice));
}

void UNoticeWidget::ShowNotice()
{
	if (!bIsUsing)
	{
		PlayAnimation(ShowAnim);
	}
}
