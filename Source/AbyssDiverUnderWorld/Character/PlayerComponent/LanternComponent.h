// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LanternComponent.generated.h"


/**
 * 플레이어의 랜턴 컴포넌트
 * 내부에서 SpotLightComponent를 생성하여 캐릭터 앞에 랜턴을 비춘다.
 * Monster가 랜턴에 노출되었는지 확인하기 위해 CapsuleComponent를 사용한다.
 * 따라서 SpotLightComponent를 외부에서 변경해서는 안 된다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API ULanternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULanternComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Method

public:
	/** Lantern Toggle 요청 */
	void RequestToggleLanternLight();
	
	/** 라이트를 생성한다. */
	void SpawnLight(USceneComponent* AttachToComponent, const float LightLength = 2000.0f);

protected:

	/** bIsLanternOn Replicate 함수 */
	UFUNCTION()
	void OnRep_bIsLanternOn();

	/** Request Toggle Lantern Light를 서버에서 처리한다. */
	UFUNCTION(Server, Reliable)
	void S_ToggleLanternLight();
	void S_ToggleLanternLight_Implementation();

	/** Actor가 라이트에 노출된 시간을 업데이트한다. */
	void UpdateExposureTimes(TArray<AActor*> OverlappedActors, float DeltaTime);

	/** Actor가 Light에 노출되었는지 확인한다. */
	bool HasActorExposedByLight(const AActor* TargetActor, const FVector& ConeOrigin, const FVector& ConeDirection, float ConeAngle, float ConeHeight) const;
	
#pragma endregion

#pragma region Variable

private:
	
	/** 현재 라이트 활성화 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsLanternOn, Category = "Character|Lantern", meta = (AllowPrivateAccess = "true"))
	uint8 bIsLanternOn : 1;

	/** 랜턴이 Offset에서 앞으로 얼마나 떨어져 있는지 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Lantern")
	float LanternForwardOffset;

	/** 랜턴의 라이트 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class USpotLightComponent> LanternLightComponent;

	/** 몬스터가 랜턴에 노출되었는지 확인하기 위한 영역을 나타내는 Capsule Component */
	UPROPERTY()
	TObjectPtr<class UCapsuleComponent> LightDetectionComponent;

	/** 몬스터가 라이트에 노출된 시간을 저장한다. Key : Monster 포인터, Value : 노출된 시간 */
	UPROPERTY()
	TMap<TObjectPtr<class AMonster>, float> MonsterExposeTimeMap;

#pragma endregion

#pragma region Getter Setter

#pragma endregion
};
