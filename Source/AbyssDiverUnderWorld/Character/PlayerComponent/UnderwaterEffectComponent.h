// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/ActorComponent.h"
#include "UnderwaterEffectComponent.generated.h"


enum class EAudioFaderCurve : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UUnderwaterEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUnderwaterEffectComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
#pragma region Method

public:
	/** 수중 효과를 활성화하거나 비활성화한다. */
	void SetEnableEffect(bool bNewEnabled);

protected:

	UFUNCTION()
	void OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);

	/** 숨쉬기 효과를 재생한다. 이 함수는 숨쉬기 효과가 활성화되었을 때만 호출된다. */
	void PlayBreathEffects();

	/** 수중에서 이동 소리를 업데이트한다. 이 함수는 수중에서 이동 중일 때만 호출된다. */
	void UpdateMovementEffects(const float DeltaTime);

	/** 급격한 속도 변화가 있을 경우에 사운드를 재생한다. */
	void CheckVelocityChange(const float DeltaTime);
	
#pragma endregion

#pragma region Variable

private:

	UPROPERTY()
	TObjectPtr<class AUnderwaterCharacter> OwnerCharacter;

	/** 수중 효과 활성화 여부. 이 값이 true일 때만 숨쉬기 효과와 이동 소리가 재생된다. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|UnderwaterEffect", meta = (AllowPrivateAccess = "true"))
	uint8 bEnabled : 1 = false;
	
	/** 숨쉬기 효과 재생 간격. 숨쉬기 간격마다 Breath Sound를 재생하고 Bubble 효과를 생성한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	float BreathInterval;

	/** 숨쉬기 효과를 재생하기 전의 첫 지연 시간. 이 시간 후에 숨쉬기 효과가 시작된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	float BreathFirstDelay;

	/** 숨쉬기 효과를 재생할 사운드. 이 사운드는 숨쉬기 효과가 활성화되었을 때 재생된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<USoundBase> BreathSound;

	/** 숨쉬기 효과를 생성할 Niagara 시스템. 이 시스템은 숨쉬기 효과를 시각적으로 표현한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<class UNiagaraSystem> BreathBubbleEffect;

	/** 숨쉬기 효과 재생 타이머 핸들. 숨쉬기 효과를 주기적으로 재생하는 데 사용된다. */
	FTimerHandle BreathEffectTimerHandle;

	// @ToDo : Sound Subsystem으로 변경

	/** 수중에서 이동 소리를 재생하고 있는지 여부. 이 값이 true일 때만 이동 소리가 재생된다. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|UnderwaterEffect", meta = (AllowPrivateAccess = "true"))
	uint8 bShouldPlayMovementSound : 1;
	
	/** 수중에서 이동 소리 재생을 위한 오디오 컴포넌트. 이 컴포넌트는 MovementSoundThreshold보다 큰 속도로 이동할 때만 소리를 재생한다. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> MovementAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SprintMovementAudioComponent;

	/** 수중에서 이동 소리 재생을 위한 사운드. 이 사운드는 MovementSoundThreshold보다 큰 속도로 이동할 때만 재생된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<USoundBase> MovementSound;

	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<USoundBase> SprintMovementSound;

	/** 수중에서 이동 소리 재생을 위한 임계값. 이 값보다 큰 속도로 이동할 때만 소리가 재생된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	float MovementSoundThreshold;

	/** 수중에서 이동 소리 재생을 위해 필요한 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	float MoveRequireTime;

	/** 수중에서 이동 소리 재생을 확인하기 위한 누적 시간 */
	float MoveTimeAccumulator;

	/** MovementSound Fade가 되는 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	float MovementSoundFadeTime;

	/** FadeOut 함수 */
	EAudioFaderCurve MovementSoundFadeCurve;

#pragma endregion

#pragma region Getter Setter

#pragma endregion
};
