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

protected:
	
	virtual void BeginPlay() override;

#pragma region Method

protected:
	
	/** 로컬 플레이어 효과를 바인딩. 다른 유저한테는 보이지 않는 효과이다. */
	void BindLocalEffects(AUnderwaterCharacter* UnderwaterCharacter);

	/** 실드 파괴 시에 호출되는 함수 */
	UFUNCTION()
	void OnShieldBroken();

	/** 실드 값이 변경될 때 호출되는 함수 */
	UFUNCTION()
	void OnShieldValueChanged(float OldShieldValue, float NewShieldValue);
	
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

	/** 실드 히트 애니메이션 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	FName ShieldHitAnimationName;

	/** 실드 히트 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|CombatEffect")
	TObjectPtr<USoundBase> ShieldHitSound;

#pragma endregion

#pragma region Getter Setter

public:
	
	/** 실드 히트 위젯을 반환한다. */
	UUserWidget* GetShieldHitWidget();
	
#pragma endregion
};
