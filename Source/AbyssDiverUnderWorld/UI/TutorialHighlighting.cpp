// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TutorialHighlighting.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "TutorialHighlighting.h"

void UTutorialHighlighting::HighlightStart(const FHighlightingInfo& Info)
{
	MovingCanvas->SetVisibility(ESlateVisibility::Visible);
	if (UCanvasPanelSlot* MovingSlot = Cast<UCanvasPanelSlot>(MovingCanvas->Slot))
	{
		//슬롯의 앵커 설정
		MovingSlot->SetAnchors(Info.Anchors);
		//슬롯 위치 설정
		MovingSlot->SetPosition(Info.Offset);
		//렌더링 트랜스폼 스케일 조정
		MovingCanvas->SetRenderScale(FVector2D(1.0f, 1.0f) * FMath::Clamp(Info.Multiply, 1.0f, Info.Multiply));
	}
	PlayAnimation(ShowHighlighting);
}

void UTutorialHighlighting::HighlightEnd()
{
	MovingCanvas->SetVisibility(ESlateVisibility::Hidden);
	if (UCanvasPanelSlot* MovingSlot = Cast<UCanvasPanelSlot>(MovingCanvas->Slot))
	{
		MovingSlot->SetPosition(FVector2D(0.0f, 0.0f));
	}
}
