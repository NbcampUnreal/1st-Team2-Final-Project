// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

	UCrosshairWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
#pragma region Method

public:

	/** 폰을 크로스헤어 위젯에 바인딩 */
	void BindWidget(APawn* Pawn);
	
protected:

	/** 환경에 따라 지연된 크로스헤어 회전을 사용할지 여부 */
	UFUNCTION()
	void OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);
	
#pragma endregion
	
#pragma region Variable

protected:

	/** Crosshair 이미지 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> CrosshairImage;

	/** Crosshair의 현재 위치 */
	FVector2D CrosshairPosition;

	/** 지연된 회전을 위한 컴포넌트 포인터 */
	TWeakObjectPtr<USceneComponent> SimulatedLaggedRotationComponent;

	/** 지연된 회전을 위한 소켓 이름 */
	FName SimulatedLaggedRotationSocketName;

	/** 지연된 회전을 사용할지 여부 */
	uint8 bIsUsingLaggedRotation : 1;

	/** 지연된 회전 보간 속도 */
	float LaggedRotationInterpSpeed;

	// (Left, Top, Right, Bottom) 순서로 여백 지정
	FVector4 DeadZoneMargin;
	
#pragma endregion
	
#pragma region Getter Setter

public:

	/** Crosshair 이미지 위젯을 반환 */
	UImage* GetCrosshairImage() const { return CrosshairImage; }

	/** Crosshair의 지연 이동 설정 */
	FORCEINLINE void SetUsingLaggedRotation(bool bNewIsUsingLaggedRotation) { bIsUsingLaggedRotation = bNewIsUsingLaggedRotation; }

	/** Crosshair 지연 이동 시뮬레이션 컴포넌트 추가 */
	FORCEINLINE void SetSimulatedLaggedRotationComponent(USceneComponent* NewComponent, FName NewSocketName = NAME_None);

	/** 크로스 헤어 위치 초기화 */
	void ResetCrosshairPosition();

#pragma endregion
	
};
