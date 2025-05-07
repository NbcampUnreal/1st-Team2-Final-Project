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

#pragma region Method

public:
	/** 초기값 설정 */
	void Initialize(int32 InitMaxHealth, int32 InitCurrentHealth, float InitMoveSpeed, int32 InitAttackPower);

	/** 데미지를 계산 */
	void TakeDamage(const float DamageAmount);
	
#pragma endregion 
	
#pragma region Variable

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	int32 MaxHealth;

	/** 현재 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	int32 CurrentHealth;

	/** 기본 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	float MoveSpeed;

	/** 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
	int32 AttackPower;
	
#pragma endregion

#pragma region Getter Setter

	/** 현재 최대 체력을 반환 */
	FORCEINLINE int32 GetMaxHealth() const { return MaxHealth; }

	/** 현재 체력을 반환 */
	FORCEINLINE int32 GetCurrentHealth() const { return CurrentHealth; }

	/** 기본 이동 속도를 반환 */
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }

	/** 공격력을 반환 */
	FORCEINLINE int32 GetAttackPower() const { return AttackPower; }
	
#pragma endregion
};
