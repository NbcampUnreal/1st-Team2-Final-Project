// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/ActorComponent.h"
#include "CombatEffectComponent.generated.h"

/**
 * 캐릭터 전투 상황 효과 컴포넌트
 * 실드 이펙트 효과 발생 시점
 * - 실드 파괴 시에 실드 파괴 이펙트 발생
 * - 실드 히트 시에 실드 히트 풀스크린 이펙트 발생
 * - 실드 파괴와 히트 동시에 발생 시에는 실드 파괴 이펙트만 발생
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UCombatEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatEffectComponent();

	UFUNCTION(Client, Reliable)
	void C_PlayShieldUseEffect();
	void C_PlayShieldUseEffect_Implementation();

protected:
	
	virtual void BeginPlay() override;

#pragma region Method

protected:
	// 현재 Replicate를 이용해서 동기화를 진행하고 있다.
	// 하지만, Replicate는 순서를 보장할 수 없으므로 추후에 문제가 생기면 Server 주도의 RPC 구현으로 변경 
	
	/** Delegate를 바인딩 한다. */
	void BindDelegate(AUnderwaterCharacter* UnderwaterCharacter);

	/** 실드 파괴 시에 호출되는 함수 */
	UFUNCTION()
	void OnShieldBroken();

	/** 실드 값이 변경될 때 호출되는 함수 */
	UFUNCTION()
	void OnShieldValueChanged(float OldShieldValue, float NewShieldValue);

	/** 데미지를 입었을 때 호출되는 함수. Server에서만 수행된다. */
	UFUNCTION()
	void OnDamageTaken(float DamageAmount, float CurrentHealth);
	
	void PlayShieldHitEffect();

	static class UWidgetAnimation* FindAnimationByName(UUserWidget* Widget, const FName& AnimationName);
	
#pragma endregion

#pragma region Variable

private:

	/** 실드 파괴 시에 사용되는 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	TObjectPtr<class UNiagaraSystem> ShieldBrokenEffect;

	/** 실드 파괴 이펙트 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> ShieldBrokenEffectComponent;

	/** 실드 파괴 사운드 */
	UPROPERTY()
	TObjectPtr<class USoundBase> ShieldBrokenSound;

	/** 실드 히트 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	TSubclassOf<UUserWidget> ShieldHitWidgetClass;

	/** 실드 풀스크린 이펙트 UI 위젯. Owner는 World이다. */
	UPROPERTY()
	TObjectPtr<UUserWidget> ShieldHitWidget;

	/** 실드 히트 시 애니메이션 */
	UPROPERTY()
	TObjectPtr<UWidgetAnimation> ShieldHitAnimation;

	/** 실드 사용 애니메이션 이름 */
	UPROPERTY()
	TObjectPtr<UWidgetAnimation> ShieldUseAnimation;

	/** 실드 히트 애니메이션 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	FName ShieldHitAnimationName;

	/** 실드 사용 애니메이션 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	FName ShieldUseAnimationName;

	/** 실드 히트 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	TObjectPtr<USoundBase> ShieldHitSound;

	/** Owner Character Cache */
	UPROPERTY()
	TObjectPtr<AUnderwaterCharacter> OwnerCharacter;

	/** 데미지 피격 시에 재생되는 블랙아웃 효과 지속 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect", meta = (ClampMin = 0.0f))
	float HitBlackoutDuration;

	/** 데미지 피격 시에 블랙아웃 효과가 페이드인되는 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect", meta = (ClampMin = 0.0f))
	float HitFadeInDuration;

	/** 데미지를 입었을 때 재생되는 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	class USoundBase* DamageTakenSound;

#pragma endregion

#pragma region Getter Setter

public:
	
	/** 실드 히트 위젯을 반환한다. */
	UUserWidget* GetShieldHitWidget();
	
#pragma endregion
};
