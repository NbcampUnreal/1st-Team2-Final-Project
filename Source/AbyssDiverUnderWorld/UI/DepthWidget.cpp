// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DepthWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


void UDepthWidget::ApplyZoneChangeToWidget(EDepthZone Old, EDepthZone New)
{
	SetZoneText(StaticEnum<EDepthZone>()->GetNameStringByValue((int64)New)); //열거형 문자열로 변환
	switch (New)
	{
		case EDepthZone::SafeZone:
			ZoneLevel = 0;
			ZoneColorImg->SetColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.1f, 1.0f));
			DepthBG->SetColorAndOpacity(FLinearColor::White);
			DepthBGSide->SetColorAndOpacity(FLinearColor::White);
			break;
		case EDepthZone::WarningZone:
			ZoneLevel = 1;
			ZoneColorImg->SetColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.0f, 1.0f));
			DepthBG->SetColorAndOpacity(FLinearColor(1.0f, 0.7f, 0.3f, 0.8f));
			DepthBGSide->SetColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.0f, 1.0f));
			break;
		case EDepthZone::DangerZone:
			ZoneLevel = 2;
			ZoneColorImg->SetColorAndOpacity(FLinearColor::Red);
			DepthBG->SetColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.6f, 1.0f));
			DepthBGSide->SetColorAndOpacity(FLinearColor::Red);
			break;
		default:
			break;
	}
}

void UDepthWidget::SetDepthText(float Depth)
{
	float ReverseDepth = -Depth;
	FString DepthString = FString::Printf(TEXT("%.1fm"), ReverseDepth);
	DepthText->SetText(FText::FromString(DepthString));
}

void UDepthWidget::SetZoneText(const FString& ZoneName)
{
	ZoneText->SetText(FText::FromString(ZoneName));
}