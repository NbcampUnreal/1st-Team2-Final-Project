// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShieldComponent.generated.h"

/** AbsorbDamage 함수의 반환값 */
USTRUCT(BlueprintType)
struct FShieldAbsorbResult
{
	GENERATED_BODY()

	FShieldAbsorbResult() 
		: AbsorbedDamage(0.0f), RemainingDamage(0.0f) {}
	
	/** 실드가 흡수한 데미지 */
	UPROPERTY(BlueprintReadOnly)
	float AbsorbedDamage;
	
	/** 실드가 흡수하고 남은 데미지 */
	UPROPERTY(BlueprintReadOnly)
	float RemainingDamage;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method

public:

	/** 실드 데미지 계산. 실드가 파괴되면 남은 데미지를 반환한다. */
	UFUNCTION(BlueprintCallable)
	FShieldAbsorbResult AbsorbDamage(const float DamageAmount);

	/** 실드를 획득. GainAmount > 0 */
	UFUNCTION(BlueprintCallable)
	void GainShield(const float GainAmount);
	
protected:

	/** 실드 값이 변경됬을 때 호출되는 블루프린트 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnShieldValueChanged"))
	void K2_OnShieldValueChanged(float NewShieldValue);

	/** 실드가 파괴되었을 때 호출되는 블루프린트 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnShieldBroken"))
	void K2_OnShiledBroken();

	/** 실드가 0에서 획득되었을 때 호출되는 블루프린트 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnShieldGained"))
	void K2_OnShieldGained();	

	/** 실드 값이 변경되었을 때 호출되는 Replicated 함수 */
	UFUNCTION()
	virtual void OnRep_ShieldValueChanged();
	
private:
	/** 내부적으로 실드 값을 설정하는 함수. NewValue를 기준으로 새로운 실드 값을 설정한다.
	 * 실드의 값은 0 이하로 내려갈 수 없다.
	 * 호출되면서 관련된 델리게이트를 호출한다.
	 * bAlwaysUpdate가 true이면 실드 값이 변경되지 않아도 델리게이트를 호출한다.
	 */
	void SetShieldValue(const float NewValue, const bool bAlwaysUpdate = false);

#pragma endregion

#pragma region Variable

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShiledValueChanged, float, ShieldValue);
	/** 실드 값이 변경되었을 때 호출되는 델리게이트 */
	FOnShiledValueChanged OnShieldValueChangedDelegate;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShiledBroken);
	/** 실드가 파괴되었을 때 호출되는 델리게이트 */
	FOnShiledBroken OnShieldBrokenDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShiledGained);
	/** 실드가 0에서 획득되었을 때 호출되는 델리게이트 */
	FOnShiledGained OnShieldGainedDelegate;

private:

	/** 실드 획득 가능 여부. 사망 시에는 Shield가 변경되면 안 된다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	uint8 bCanGainShield : 1;
	
	/** 현재 실드 값 */
	UPROPERTY(ReplicatedUsing="OnRep_ShieldValueChanged", EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	float ShieldValue;

	/** 이전 실드 값. Client에서 Shield 변화를 확인하기 위한 이전 ShieldValue */
	float OldShieldValue;
	
#pragma endregion

#pragma region Getter Setter

public:
	
	/** 실드 획득 가능한 여부를 반환 */
	FORCEINLINE bool CanGainShield() const { return bCanGainShield; }

	UFUNCTION(BlueprintCallable)
	/** 실드를 가지고 있는 여부를 반환 */
	FORCEINLINE bool HasShield() const { return ShieldValue > 0.0f; }

	/** 현재 실드 값을 반환 */
	FORCEINLINE float GetShieldValue() const { return ShieldValue; }
	
#pragma endregion
	
};
