// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OxygenComponent.generated.h"


UENUM(BlueprintType)
enum class EOXygenChangeResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	SystemDisabled UMETA(DisplayName = "SystemDisabled"),
	NotAuthority UMETA(DisplayName = "NotAuthority"),
	BlockedByLimit UMETA(DisplayName = "BlockedByLimit"),
};

/** 현재 산소량과 최대 산소량이 개별적으로 Replicate 되었을 경우 최대 산소량을 변경할 때 일시적으로 Oxygen 퍼센티지가 일치하지 않는 현상이 발생한다.
 * 해당 현상을 방지하기 위해서 2개의 변수를 동시에 동기화한다.
 */
USTRUCT(BlueprintType)
struct FOxygenState
{
	GENERATED_BODY()

	FOxygenState() : MaxOxygenLevel(600.0f), OxygenLevel(600.0f) {};
	
	/** 최대 산소량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxOxygenLevel;

	/** 현재 산소량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float OxygenLevel;
};

// To-DO
// 1. Timer를 이용해서 소모하는 방식으로 변경
// 2. 산소 소모량을 깊이에 따라 변화하도록 한다.

// Require System
// 현재 수심을 확인할 수 있는 System이 필요하다.

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UOxygenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOxygenComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
#pragma region Method

protected:
	/** 현재 산소량이 변경됬을 떄 호출 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnOxygenLevelChanged"))
	void K2_OnOxygenLevelChanged(float CurrentOxygenLevel, float MaxOxygenLevel);

	/** 현재 산소량이 모두 소모되었을 때 호출 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnOxygenDepleted"))
	void K2_OnOxygenDepleted();

	/** 산소량이 모두 소모되었다가 회복되었을 때 호출 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnOxygenRestored"))
	void K2_OnOxygenRestored();

	UFUNCTION()
	void OnRep_OxygenStateChanged();
	
private:
	/** 매 틱마다 산소를 소모 */
	void ConsumeOxygen(float DeltaTime);
	
#pragma endregion
	
#pragma region Varaible

public:
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOxygenLevelChanged, float, CurrentOxygenLevel, float, MaxOxygenLevel);
	/** 산소량이 변경되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOxygenLevelChanged OnOxygenLevelChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOxygenDepleted);
	/** 산소가 전부 소모되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOxygenDepleted OnOxygenDepleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOxygenRestored);
	/** 산소가 고갈되었다가 회복될 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOxygenRestored OnOxygenRestored;
	
private:
	/** 산소 시스템 활성화 여부. 비활성화 되면 산소 회복, 소모를 정지한다. 사망 상태 등에서 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	uint8 bOxygenSystemEnabled : 1;
	
	/** 산소 소모 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat", meta=(AllowPrivateAccess = "true"))
	uint8 bShouldConsumeOxygen : 1;
	
	UPROPERTY(ReplicatedUsing="OnRep_OxygenStatechanged", EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	FOxygenState OxygenState;

	/** Client에서 OxygenRestore을 검사하기 위한 이전 OxygenLevel */
	float OldOxygenLevel;

	/** 산소 소모량, 깊이에 따라 변화하지만 현재는 고정값을 소모하도록 한다. 양수 값을 입력으로 한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat", meta=(AllowPrivateAccess = "true", ClampMin="0.0"))
	float OxygenConsumeRate;

#pragma endregion

#pragma region Getter Setter

public:
	void SetOxygenSystemEnabled(bool bNewOxygenSystemEnabled);
	
	/** 산소 시스템 활성화 여부, 비활성화 되면 산소 회복, 소모를 정지한다. */
	FORCEINLINE bool IsOxygenSystemEnabled() const { return bOxygenSystemEnabled; }

	/** 산소 소모 여부. 수중에서는 소모하고 지상에서는 소모하지 않는다. */
	UFUNCTION(BlueprintCallable)
	void SetShouldConsumeOxygen(bool bNewShouldConsumeOxygen);

	/** 산소 소모 여부 */
	FORCEINLINE bool ShouldConsumeOxygen() const { return bShouldConsumeOxygen; }

	/** 최대 산소량 */
	FORCEINLINE float GetMaxOxygenLevel() const { return OxygenState.MaxOxygenLevel; }
	
	/** 최대 산소량을 설정한다. */
	UFUNCTION(BlueprintCallable)
	void SetMaxOxygenLevel(float NewMaxOxygenLevel);

	/** 현재 산소량 */
	FORCEINLINE float GetOxygenLevel() const { return OxygenState.OxygenLevel; }

	/** 현재 산소량이 0보다 작거나 같으면 true를 반환한다. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE float IsOxygenDepleted() const { return OxygenState.OxygenLevel <= 0; }

	/** 산소를 충전 */
	UFUNCTION(BlueprintCallable)
	EOXygenChangeResult RefillOxygen(float RefillAmount);

private:
	/** 산소량을 설정한다. 산소량은 0보다 작거나 MaxOxygenLevel보다 클 수 없다.
	 * 산소량이 변경되면 OnOxygenLevelChanged를 호출한다.
	 * 산소량이 모두 소모되면 OnOxygenDepleted를 호출한다.
	 * */
	void SetOxygenLevel(float NextOxygenLevel, bool bAlwaysUpdate = false);
	
#pragma endregion
};
