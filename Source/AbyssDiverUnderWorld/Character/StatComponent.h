// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method

public:
	/** 초기값 설정 */
	void Initialize(int32 InitMaxHealth, int32 InitCurrentHealth, float InitMoveSpeed, int32 InitAttackPower);

	/** 데미지를 계산 */
	void TakeDamage(const float DamageAmount);

	/** 체력을 회복한다. */
	void RestoreHealth(const float RestoreAmount);

protected:
	/** 최대 체력 레플리케이트 함수 */
	UFUNCTION()
	void OnRep_MaxHealth();

	/** CurrentHealth 레플리케이트 함수 */
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Timer에서 체력 회복 함수 */
	virtual void RegenHealth();

private:
	/** 체력 회복을 정지한다. */
	void StopHealthRegen();

	/** 체력 회복이 동작 중인지 확인 */
	bool IsHealthRegenActive() const;
	
#pragma endregion 
	
#pragma region Variable

public:

	/** 체력 변경 시 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, CurrentHealth, int32, MaxHealth);
	UPROPERTY(BlueprintAssignable, Category="Stat")
	FOnHealthChanged OnHealthChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveSpeedChanged, float, NewMoveSpeed);
	/** MoveSpeed가 변경될 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category="Stat")
	FOnMoveSpeedChanged OnMoveSpeedChanged;
	
	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category="Stat")
	int32 MaxHealth;

	/** 현재 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category="Stat")
	int32 CurrentHealth;

	/** 기본 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	float MoveSpeed;

	/** 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	int32 AttackPower;

private:
	/** 체력 회복 Timer */
	FTimerHandle HealthRegenTimerHandle;
	
	/** 초당 체력 회복 속도 */
	float HealthRegenRate;

	/** 체력 회복 타이머 동작 주기 */
	float HealthUpdateInterval;
	
	/** 체력 회복 후에 남은 소수점 값 */
	float HealthAccumulator;

#pragma endregion

#pragma region Getter Setter

public:
	
	/** 현재 최대 체력을 반환 */
	FORCEINLINE int32 GetMaxHealth() const { return MaxHealth; }

	/** 현재 체력을 반환 */
	FORCEINLINE int32 GetCurrentHealth() const { return CurrentHealth; }

	/** 기본 이동 속도를 반환 */
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }

	/** 공격력을 반환 */
	FORCEINLINE int32 GetAttackPower() const { return AttackPower; }

	/** 기준 속도를 설정 */
	FORCEINLINE void SetMoveSpeed(const float NewMoveSpeed);

	/** 무분별하게 Health Regen Rate에 접근하고 수정을 하면 값을 잃어버릴 수 있다.
	 * Regen Rate에 접근하는 객체가 많아지면 구조를 변경해야 한다.
	 * 만약에 여러 객체에서 접근을 하게 된다면 Modifier를 정의해서 Modifer를 적용 / 비적용 상태로 구현한다.
	 */

	/** 초당 회복량을 반환 */
	FORCEINLINE float GetHealthRegenRate() const { return HealthRegenRate; }

	/** 초당 회복량을 설정 */
	void SetHealthRegenRate(float NewHealthRegenRate);

	/** 초당 회복량을 추가 */
	void AddHealthRegenRate(float AddHealthRegenRate);
	
#pragma endregion
};
