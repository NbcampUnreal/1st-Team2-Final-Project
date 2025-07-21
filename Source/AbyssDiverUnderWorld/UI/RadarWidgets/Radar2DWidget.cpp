#include "UI/RadarWidgets/Radar2DWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Interactable/OtherActors/Radars/Radar2DComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"

#include "UI/RadarWidgets/RadarReturnWidget.h"
#include "Character/UnderwaterCharacter.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/CanvasPanelSlot.h"

void URadar2DWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ensureMsgf(ReturnWidgetClass, TEXT("ReturnWidgetClass가 등록되지 않았습니다.")) == false)
	{
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(RadarOverlay->Slot))
	{
		// 레이더 오버레이는 정사각형이라 가정
		RadarWidgetRadius = CanvasSlot->GetSize().X / 2.0f;
	}

	ActivatedReturnWidgets.Empty();
	DeactivatedReturnImages.Empty();

	const int32 ChildCount = RadarOverlay->GetChildrenCount();

	for (int32 i = 0; i < ChildCount; ++i)
	{
		const int32 ChildIndex = ChildCount - i - 1;
		UWidget* Child = RadarOverlay->GetChildAt(ChildIndex);
		if (Child->IsA<URadarReturnWidget>())
		{
			RadarOverlay->RemoveChildAt(ChildIndex);
			Child->RemoveFromParent();
		}
	}
}

void URadar2DWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (IsValid(PC) == false || PC->IsPendingKillPending())
	{
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PC->GetPawn());
	if (IsValid(PlayerCharacter) == false || PlayerCharacter->IsPendingKillPending())
	{
		return;
	}

	URadar2DComponent* Radar = PlayerCharacter->GetComponentByClass<URadar2DComponent>();
	if (IsValid(Radar) == false || Radar->IsBeingDestroyed())
	{
		return;
	}

	OwnerLocation2D = FVector2D(PlayerCharacter->GetActorLocation());
	OwnerRotation = PlayerCharacter->GetActorRotation();

	const TSet<TObjectPtr<URadarReturn2DComponent>>& AvailableRadarReturns = Radar->GetAllReturnsInDetectRadius();
	ActualRadarRadius = Radar->GetRadarDetectRadius();

	for (auto It = ActivatedReturnWidgets.CreateIterator(); It; ++It)
	{
		if (AvailableRadarReturns.Contains(It->Key) == false)
		{
			DespawnReturnWidget(It->Value);
			ActivatedReturnWidgets.Remove(It->Key);
		}
	}

	for (URadarReturn2DComponent* RadarReturn : AvailableRadarReturns)
	{
		UpdateReturn(RadarReturn);
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(RadarOverlay->Slot))
	{
		RadarOverlay->SetRenderTransformAngle(-OwnerRotation.Yaw - 90);
	}
}

void URadar2DWidget::AddReturn(URadarReturn2DComponent* ReturnComponent)
{
	if (HasReturn(ReturnComponent))
	{
		return;
	}

	TObjectPtr<URadarReturnWidget> NewReturnWidget = nullptr;
	if (DeactivatedReturnImages.Dequeue(NewReturnWidget) && IsValid(NewReturnWidget))
	{
		NewReturnWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		SetProperReturnColor(ReturnComponent, NewReturnWidget);
		ActivatedReturnWidgets.Add({ ReturnComponent, NewReturnWidget });
	}
	else
	{
		SpawnReturn(ReturnComponent);
	}
}

void URadar2DWidget::SpawnReturn(URadarReturn2DComponent* ReturnComponent)
{
	if (ReturnWidgetClass == nullptr)
	{
		LOGV(Error, TEXT("ReturnImageClass is not set"));
		return;
	}

	TObjectPtr<URadarReturnWidget> NewReturnWidget = CreateWidget<URadarReturnWidget>(this, ReturnWidgetClass);
	check(NewReturnWidget);

	UOverlaySlot* RadarOverlaySlot = RadarOverlay->AddChildToOverlay(NewReturnWidget);
	if (RadarOverlaySlot == nullptr)
	{
		LOGV(Error, TEXT("Overlay Slot Is Not Valid"));
		return;
	}

	RadarOverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	RadarOverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

	SetReturnWidgetTransform(ReturnComponent, NewReturnWidget);

	NewReturnWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetProperReturnColor(ReturnComponent, NewReturnWidget);
	ActivatedReturnWidgets.Add({ ReturnComponent, NewReturnWidget });
}

void URadar2DWidget::DespawnReturn(const URadarReturn2DComponent* ReturnComponent)
{
	if (HasReturn(ReturnComponent) == false)
	{
		return;
	}

	URadarReturnWidget* ReturnWidget = ActivatedReturnWidgets[ReturnComponent];
	if (IsValid(ReturnWidget))
	{
		DespawnReturnWidget(ReturnWidget);
	}

	ActivatedReturnWidgets.Remove(ReturnComponent);
}

void URadar2DWidget::DespawnReturnWidget(URadarReturnWidget* ReturnWidget)
{
	DeactivatedReturnImages.Enqueue(ReturnWidget);
	ReturnWidget->SetVisibility(ESlateVisibility::Hidden);
}

void URadar2DWidget::UpdateReturn(URadarReturn2DComponent* ReturnComponent)
{
	if (IsValid(ReturnComponent) == false || ReturnComponent->IsBeingDestroyed())
	{
		DespawnReturn(ReturnComponent);
		return;
	}

	if (HasReturn(ReturnComponent) == false)
	{
		AddReturn(ReturnComponent);
	}

	URadarReturnWidget* CurrentReturnWidget = ActivatedReturnWidgets[ReturnComponent];
	if (IsValid(CurrentReturnWidget) == false)
	{
		LOGV(Error, TEXT("Return Image Is Not Valid"));
		return;
	}

	SetReturnWidgetTransform(ReturnComponent, CurrentReturnWidget);
	
}

bool URadar2DWidget::HasReturn(const URadarReturn2DComponent* ReturnComponent)
{
	return ActivatedReturnWidgets.Contains(ReturnComponent);
}

void URadar2DWidget::SetProperReturnColor(URadarReturn2DComponent* ReturnComponent, URadarReturnWidget* ReturnWidget)
{
	if (IsValid(ReturnWidget) == false)
	{
		return;
	}

	EReturnForceType ReturnForceType = ReturnComponent->GetReturnForceType();

	FColor ReturnColor;

	switch (ReturnForceType)
	{
	case EReturnForceType::Friendly:
		ReturnColor = FColor::Green;
		break;
	case EReturnForceType::Hostile:
		ReturnColor = FColor::Red;
		break;
	case EReturnForceType::Neutral:
		ReturnColor = FColor::White;
		break;
	default:
		check(false);
		return;
	}

	ReturnWidget->SetReturnImageColor(ReturnColor);
}

void URadar2DWidget::SetReturnWidgetTransform(URadarReturn2DComponent* ReturnComponent, URadarReturnWidget* ReturnWidget)
{
	if (IsValid(ReturnComponent) == false || ReturnComponent->IsBeingDestroyed())
	{
		return;
	}

	AActor* TargetReturn = ReturnComponent->GetOwner();
	if (IsValid(TargetReturn) == false || TargetReturn->IsPendingKillPending())
	{
		return;
	}

	FVector2D ReletiveReturnPosition(TargetReturn->GetActorLocation());
	ReletiveReturnPosition -= OwnerLocation2D;
	FVector2D ReturnPositionInRadar = ReletiveReturnPosition * RadarWidgetRadius / ActualRadarRadius;

	if (ReturnComponent->GetAlwaysDisplay() && ReturnPositionInRadar.Length() >= RadarWidgetRadius)
	{
		ReturnPositionInRadar.Normalize();
		ReturnPositionInRadar *= RadarWidgetRadius;
	}

	ReturnWidget->SetRenderTranslation(ReturnPositionInRadar);
	ReturnWidget->SetRenderScale(FVector2D(ReturnComponent->GetReturnScale()));
}
