// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

// @TODO : Stamina Refactoring
// 러프하게 대략적인 스태미나 시스템을 구현한 컴포넌트입니다.
// 추후 구현 사항으로 Prediction이나 Stamina의 정확한 소모량을 계산하도록 한다.

USTRUCT(BlueprintType)
struct FStaminaStatus
{
	GENERATED_BODY()

	FStaminaStatus() : MaxStamina(600.f), Stamina(600.f)
	{
	}

	/** Stamina의 최대 수치 */
	UPROPERTY(BlueprintReadWrite)
	float MaxStamina;

	/** Stamina의 현재 수치 */
	UPROPERTY(BlueprintReadWrite)
	float Stamina;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method

public:
	/** Sprint 시작 요청, Stamina가 감소하기 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void RequestStartSprint();

	/** Sprint 종료 요청, Stamina 회복이 시작된다. */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void RequestStopSprint();

protected:
	/** Sprint 시작, Stamina가 감소하기 시작한다. */
	virtual void StartSprint();
	
	/** Sprint 종료, Stamina 회복이 시작된다. */
	virtual void StopSprint();

	/** Stamina 변화가 있을 떄 호출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stat", meta = (DisplayName = "OnStaminaChanged"))
	void K2_OnStaminaChanged(float MaxStamina, float Stamina);

	/** Sprint 상태 변화가 있을 떄 호출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stat", meta = (DisplayName = "OnSprintStateChanged"))
	void K2_OnSprintStateChanged(bool bNewSprinting);
	
private:
	/** Stamina OnRep 함수 */
	UFUNCTION()
	void OnRep_StaminaStatusChanged();

	/** Sprint 상태 OnRep 함수 */
	UFUNCTION()
	void OnRep_IsSprintingChanged();

	/** 서버에서 Sprint 시작 요청을 받았을 때 호출되는 함수 */
	UFUNCTION(Server, Reliable)
	void S_StartSprint();
	void S_StartSprint_Implementation();

	/** 서버에서 Sprint 종료 요청을 받았을 때 호출되는 함수 */
	UFUNCTION(Server, Reliable)
	void S_StopSprint();
	void S_StopSprint_Implementation();

	/** Sprint 도중 Stamina를 소모하는 함수 */
	void ConsumeStamina();
	
	/** Timer 중에 Stamina 자연 회복 함수 */
	void RegenerateStamina();
	
	/** Stamina 자연 회복을 시작하는 함수 */
	void StartRegenerateStamina();

#pragma endregion

#pragma region Variable

public:
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, Stamina, float, MaxStamina);
	/** Stamina의 현재 값이 변경되었을 떄 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnStaminaChanged OnStaminaChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSprintStateChanged, bool, bNewSprinting);
	/** Sprint 상태가 변경되었을 떄 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnSprintStateChanged OnSprintStateChanged;
	
private:
	
	/** Stamina 업데이트 주기 */
	UPROPERTY(EditDefaultsOnly, Category = "Stat", meta = (ClampMin = "0.0", AllowPrivateAccess= "true"))
	float StaminaUpdateInterval;

	/** 현재 Sprint 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing="OnRep_IsSprintingChanged", Category = "Stat", meta = (AllowPrivateAccess = "true"))
	uint8 bIsSprinting : 1;

	/** 현재 Stamina 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing="OnRep_StaminaStatusChanged", Category = "Stat", meta = (AllowPrivateAccess= "true"))
	FStaminaStatus StaminaStatus;

	/** Sprint 도중 Stamina 초당 소모량. 양수 범위이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (ClampMin = "0.0", AllowPrivateAccess= "true"))
	float SprintConsumeRate;

	/** Sprint 도중 Stamina 초당 회복량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (ClampMin = "0.0", AllowPrivateAccess= "true"))
	float SprintRegenRate;

	/** Sprint 도중 Stamina 회복 대기 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (ClampMin = "0.0", AllowPrivateAccess= "true"))
	float StaminaRegenDelay;
	
	/** Stamina 컴포넌트에서 통합적으로 사용하는 Timer Handle.
	* Stamina의 회복, 대기, 소모 상태는 모두 이 Timer Handle을 사용한다.
	* 이는 Stamina 회복, 대기, 소모는 동시에 이루어질 수 없기 때문이다.
	*/
	FTimerHandle StaminaTimeHandle;

#pragma endregion

#pragma region Getter Setter

public:
	/** Stamina 컴포넌트 초기화 함수 */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void InitStamina(float MaxStamina, float Stamina);

	/** Sprint 상태 반환 */
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

	/** 최대 Stamina 설정. 현재 Stamina보다 작은 값을 설정하면 현재 Stamina는 최대 Stamina가 된다. */
	UFUNCTION(BlueprintCallable)
	void SetMaxStamina(const float NewMaxStamina);

	/** 최대 Stamina 반환 */
	FORCEINLINE float GetMaxStamina() const { return StaminaStatus.MaxStamina; }

	/** 현재 Stamina 반환 */
	FORCEINLINE float GetStamina() const { return StaminaStatus.Stamina; }

	/** Sprint 가능 여부 반환 */
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool CanSprint() const { return StaminaStatus.Stamina > 0.f; }

	
private:
	/** Stamina 수치를 설정한다. [0, MaxStamina] 범위로 설정된다. Stamina 변화 이벤트를 발생한다. */
	void SetStamina(const float NewStamina);
	
#pragma endregion
};
