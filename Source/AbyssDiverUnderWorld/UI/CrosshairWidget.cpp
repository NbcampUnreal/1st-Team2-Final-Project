// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/Image.h"
#include "GameFramework/SpringArmComponent.h"

UCrosshairWidget::UCrosshairWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CrosshairPosition = FVector2D::ZeroVector;
	bIsUsingLaggedRotation = false;
	LaggedRotationInterpSpeed = 20.0f;
	DeadZoneMargin = FVector4(0.0f, 0.0f, 0.0f, 0.0f);
}

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateCanTick();
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetOwningPlayerPawn() == nullptr)
	{
		return;
	}
	
	int32 ViewPortSizeX, ViewPortSizeY;
	GetOwningPlayer()->GetViewportSize(ViewPortSizeX, ViewPortSizeY);
	FVector2D ViewportCenter = FVector2D(0.5f * ViewPortSizeX, 0.5f * ViewPortSizeY);
    
	// 기본 타겟은 화면 중앙
	FVector2D TargetPosition = ViewportCenter;
    
	if (bIsUsingLaggedRotation && SimulatedLaggedRotationComponent.IsValid())
	{
		// @ToDo : SimulatedLaggedRotation을 이용하지 않고 Control Rotation을 이용한 크로스헤어 회전 구현
		if (USceneComponent* LaggedRotationComponent = SimulatedLaggedRotationComponent.Pin().Get())
		{
			const FVector CameraLocation = GetOwningPlayerCameraManager()->GetCameraLocation();
			FRotator LaggedRotation = LaggedRotationComponent->GetSocketRotation(SimulatedLaggedRotationSocketName);

			FVector CameraForward = GetOwningPlayerCameraManager()->GetCameraRotation().Vector();
			FVector LoaggedLookForward = LaggedRotation.Vector();
			
			float Dot = FVector::DotProduct(CameraForward, LoaggedLookForward);
			float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));
			
			if (AngleDeg > 60.0f)
			{
				FVector ClampedLook = FMath::Lerp(CameraForward, LoaggedLookForward, 60.0f / AngleDeg).GetSafeNormal();
				LaggedRotation = ClampedLook.Rotation();
			}
			
			const FVector LaggedLookTarget = CameraLocation + LaggedRotation.Vector() * 1000.0f;
			
			if (!GetOwningPlayer()->ProjectWorldLocationToScreen(LaggedLookTarget, TargetPosition))
			{
				TargetPosition = CrosshairPosition; // 실패 시 현재 위치 유지
			}
		}
	}
    
	// 화면 중앙 기준 오프셋 계산
	FVector2D Offset = TargetPosition - ViewportCenter;
    
	// 보간 적용
	Offset = FMath::Vector2DInterpTo(CrosshairPosition, Offset, InDeltaTime, LaggedRotationInterpSpeed);

	// 데드존은 사각형으로 위치가 제한되기 때문에 부자연스럽게 보인다.
	// 데드존 적용 방식을 개선할 것
	// 데드존 적용 (상대 좌표계에 맞게)
	// const float DeadZoneLeft = DeadZoneMargin.X;
	// const float DeadZoneTop = DeadZoneMargin.Y;
	// const float DeadZoneRight = DeadZoneMargin.Z;
	// const float DeadZoneBottom = DeadZoneMargin.W;
     
	// // 최대 이동 범위 제한
	// const float MaxOffsetX = (ViewPortSizeX / 2.0f) - DeadZoneRight;
	// const float MaxOffsetY = (ViewPortSizeY / 2.0f) - DeadZoneBottom;
     
	// Offset = FVector2D(
	// 	FMath::Clamp(Offset.X, -MaxOffsetX + DeadZoneLeft, MaxOffsetX),
	// 	FMath::Clamp(Offset.Y, -MaxOffsetY + DeadZoneTop, MaxOffsetY)
	// );
    
	CrosshairPosition = Offset; // 현재 위치 저장

	// 위젯 내부에서 크로스헤어 이미지만 이동
	if (CrosshairImage)
	{
		CrosshairImage->SetRenderTranslation(Offset);
	}
}

void UCrosshairWidget::BindWidget(APawn* Pawn)
{
	AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	SetSimulatedLaggedRotationComponent(Character->GetMesh1PSpringArm(), Character->GetMesh1PSpringArm()->SocketName);
	SetUsingLaggedRotation(Character->GetEnvironmentState() == EEnvironmentState::Underwater);
	Character->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UCrosshairWidget::OnEnvironmentStateChanged);

	// Dynamic Delegate이기 때문에 UFUNCTION만 사용 가능한 상태이다.
	Character->OnEmoteStartDelegate.AddDynamic(this, &UCrosshairWidget::HideCrosshair);
	Character->OnEmoteEndDelegate.AddDynamic(this, &UCrosshairWidget::ShowCrosshair);
}

void UCrosshairWidget::OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState,
	EEnvironmentState NewEnvironmentState)
{
	SetUsingLaggedRotation(NewEnvironmentState == EEnvironmentState::Underwater);
}

void UCrosshairWidget::SetSimulatedLaggedRotationComponent(USceneComponent* NewComponent, FName NewSocketName)
{
	SimulatedLaggedRotationComponent = TWeakObjectPtr<USceneComponent>(NewComponent);;
	SimulatedLaggedRotationSocketName = NewSocketName;
}

void UCrosshairWidget::ResetCrosshairPosition()
{
	int32 ViewPortSizeX, ViewPortSizeY;
	GetOwningPlayer()->GetViewportSize(ViewPortSizeX, ViewPortSizeY);
	CrosshairPosition = FVector2D(0.5f * ViewPortSizeX, 0.5f * ViewPortSizeY);
}

void UCrosshairWidget::ShowCrosshair()
{
	if (CrosshairImage)
	{
		CrosshairImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCrosshairWidget::HideCrosshair()
{
	if (CrosshairImage)
	{
		CrosshairImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
