// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "CameraEffectComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UCameraEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCameraEffectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

#pragma region Method

public:

	/** 원형 Blur 효과를 페이드 인/아웃 시작 */
	UFUNCTION(BlueprintCallable)
	void StartRadialBlurFade(bool bIsFadeIn);

protected:
	/** Radial Blur 효과를 Camera에 적용. 이미 적용되어 있다면 아무것도 하지 않는다. */
	void ApplyRadialBlurEffect();

	/** Radial Blur 효과 제거 */
	UFUNCTION(BlueprintCallable)
	void RemoveRadialBlur();

	/** Radial Blur Fade 효과 업데이트 */
	void UpdateRadialBlur();
	
#pragma endregion

#pragma region Variable

protected:

	/** Radial Blur 머티리얼 인스턴스 */
	UPROPERTY(EditAnywhere, Category = "PostProcess")
	TObjectPtr<UMaterialInterface> RadialBlurMaterial;

	/** Radial Blur 머티리얼 인스턴스 다이나믹. 현재는 효과 적용 시에 생성을 하고 효과 해제 시에 파괴된다. */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> RadialBlurMID;

	/** Radial Blur 머티리얼 파라미터 이름. 기본값이 0.0f로 설정되어 있어야 한다. */
	FName RadialBlurSpreadParam = TEXT("SpreadAmount");
	
	/** Radial Blur Fade 지속 속도. */
	UPROPERTY(EditDefaultsOnly, Category = "PostProcess")
	float RadialBlurFadeSpeed = 5.0f;

	/** Radial Blur 목표 알파 값 */
	float RadialBlurTargetAlpha = 0.0f;

	/** 현재 Radial Blur 알파 값 */
	float RadialBlurAlpha = 0.0f;

	/** Fade In 시에 적용될 최종 Blur 값 */
	UPROPERTY(EditDefaultsOnly, Category = "PostProcess", meta = (ClampMin = "0.001"))
	float RadialBlurTargetValue = 2.0f;

	/** 원형 Blur 업데이트 타이머 핸들 */
	FTimerHandle RadialBlurTimerHandle;

	/** Radial Blur Fade In/Out 업데이트 간격 */
	UPROPERTY(EditDefaultsOnly, Category = "PostProcess")
	float RadialBlurUpdateInterval = 0.016f;
	
#pragma endregion
};
