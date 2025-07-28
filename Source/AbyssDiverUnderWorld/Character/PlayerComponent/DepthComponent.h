// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable/Item/ADUseItem.h"
#include "DepthComponent.generated.h"


enum EDepthZone : int;

/**
 * Actor의 깊이를 관리하는 컴포넌트
 * 게임 Z 축을 기점으로 깊이를 계산한다.
 * 깊이에 따라 안전 구역, 경고 구역, 위험 구역으로 구분한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UDepthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDepthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void BeginPlay() override;

#pragma region Method

protected:

	/** 깊이로부터 DepthZone을 결정한다. */
	EDepthZone DetermineZone(float CurrentZ) const;
	
	/** 현재 깊이 구역을 갱신한다. */
	void UpdateZone();

	/** 지상 상태로 가면 깊이 구역을 SafeZone으로 설정한다. */
	UFUNCTION()
	void OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);
	
private:

	/** 맵의 깊이 정보를 받아서 설정한다. */
	void GetMapDepthData();

#pragma endregion
	
#pragma region Variable

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDepthZoneChanged, EDepthZone, OldDepthZone, EDepthZone, NewDepthZone);
	/** 깊이 구역이 변경되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Depth")
	FOnDepthZoneChanged OnDepthZoneChangedDelegate;

private:

	/** 현재 수심 컴포넌트가 활성화되어 있는지 여부
	 * 비활성화되면 항상 SafeZone으로 간주한다.
	 */
	uint8 bIsActive : 1;
	
	/** 맵에서 기점이 되는 Z 좌표 */
	float ReferenceZ;

	/** ReferenceZ의 게임적 깊이 값 */
	float ReferenceDepth;

	/** 경고 구역의 Z축 값, World Map을 기준으로 한다. */
	float WarningZoneZ;

	/** 위험 구역의 Z축 값, World Map을 기준으로 한다. */
	float DangerZoneZ;

	/** 현재 Actor의 깊이 구역 */
	EDepthZone DepthZone;
	
#pragma endregion
	
#pragma region Getter Setter

public:
	
	/** 현재 DepthComponent 활성화를 설정한다. */
	void SetDepthZoneActive(bool bActive);
	
	/** Actor의 현재 깊이를 반환한다.
	 * 깊이는 Actor의 Z 위치를 기준으로 계산되고 양수이다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Depth")
	float GetDepth() const;

	UFUNCTION(BlueprintCallable, Category = "Depth")
	EDepthZone GetDepthZone() const { return DepthZone; }
	
#pragma endregion
};
