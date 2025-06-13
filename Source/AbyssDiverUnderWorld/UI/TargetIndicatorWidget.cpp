#include "UI/TargetIndicatorWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"
#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"

#include "Components/Overlay.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

void UTargetIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CalcAndSetCorrectTargetPosition();
}

void UTargetIndicatorWidget::InitWidget(ATargetIndicatorManager* NewOwningManager)
{
	OwningManager = NewOwningManager;
}

void UTargetIndicatorWidget::ChangeTargetPosition(const FVector2D& NewPosition)
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TargetVerticalBox->Slot);
	if (CanvasSlot == nullptr)
	{
		LOGV(Error, TEXT("CanvasSlot == nullptr"));
		return;
	}

	CanvasSlot->SetPosition(NewPosition);
}

void UTargetIndicatorWidget::ChangeTargetImage(UTexture2D* NewIcon)
{
	TargetImage->SetBrushFromTexture(NewIcon);
}

void UTargetIndicatorWidget::ChangeTargetDistanceText(int32 MeterDistance)
{
	FString NewDistanceString = FString::FromInt(MeterDistance);
	NewDistanceString += TEXT("m");

	DistanceText->SetText(FText::FromString(NewDistanceString));
}

void UTargetIndicatorWidget::CalcAndSetCorrectTargetPosition()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LOGV(Error, TEXT("World == nullptr"));
		SetVisible(false);
		return;
	}

	FVector TargetLocation;
	if (OwningManager->TryGetTargetLocation(TargetLocation) == false)
	{
		SetVisible(false);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr)
	{
		LOGV(Error, TEXT("PC == nullptr"));
		SetVisible(false);
		return;
	}

	APawn* Player = PC->GetPawn();
	if (Player == nullptr)
	{
		LOGV(Error, TEXT("Player == nullptr"));
		SetVisible(false);
		return;
	}

	FVector2D TargetScreenPos;
	bool bIsOnScreen = PC->ProjectWorldLocationToScreen(TargetLocation, TargetScreenPos, true);
	FVector PlayerLocation = Player->GetActorLocation();

	FVector DirectionToTarget = PlayerLocation - TargetLocation;
	float DistanceToTarget = DirectionToTarget.Length();

	DirectionToTarget.Normalize();

	SetVisible(bIsOnScreen);

	UGameViewportClient* Viewport = World->GetGameViewport();
	if (Viewport == nullptr)
	{
		LOGV(Error, TEXT("Viewport == nullptr"));
		return;
	}

	FVector2D ViewportSize;
	Viewport->GetViewportSize(ViewportSize);

	TargetScreenPos.X = FMath::Clamp(TargetScreenPos.X, 0, ViewportSize.X);
	TargetScreenPos.Y = FMath::Clamp(TargetScreenPos.Y, 0, ViewportSize.Y);

	ChangeTargetPosition(TargetScreenPos);
	ChangeTargetDistanceText(DistanceToTarget / 100.0f); // 미터 단위
}

void UTargetIndicatorWidget::SetVisible(bool bShouldVisible)
{
	if (bShouldVisible)
	{
		TargetVerticalBox->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		TargetVerticalBox->SetVisibility(ESlateVisibility::Hidden);
	}
}