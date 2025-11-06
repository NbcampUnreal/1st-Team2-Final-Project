// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/ActorComponent.h"
#include "UnderwaterEffectComponent.generated.h"


enum class EAudioFaderCurve : uint8;
enum class ESFX : uint8;
class USoundSubsystem;

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

	/** Start Combat sound */
	void StartCombatEffect();

	/** Stop Combat sound */
	void StopCombatEffect();

protected:
	/** 환경 상태가 변경되었을 때 호출되는 함수. 수중 상태일 때만 효과를 활성화한다. */
	UFUNCTION()
	void OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);

	/** 사망 상태가 되었을 때 호출되는 함수. 사망 중에는 효과를 중지한다. */
	UFUNCTION()
	void OnDeath();

	/** 피해를 받았을 때 호출되는 함수. 숨소리 간격을 조정하기 위해 사용한다. */
	UFUNCTION()
	void OnDamageTaken(float DamageAmount, float CurrentHealth);

	/** 캐릭터가 넉백되었을 떄 호출되는 함수. 넉백 속도를 받아 숨쉬기 효과를 초기화한다. */
	UFUNCTION()
	void OnKnockback(FVector KnockbackVelocity);

	/** 넉백이 끝났을 때 호출되는 함수. 숨쉬기 효과를 재개한다. */
	UFUNCTION()
	void OnKnockbackEnd();

	/** 캡쳐 상태가 시작되었을 때 호출되는 함수. 숨쉬기 효과를 중지한다. */
	UFUNCTION()
	void OnCaptureStart();

	/** 캡쳐 상태가 종료되었을 때 호출되는 함수. 숨쉬기 효과를 재개한다. */
	UFUNCTION()
	void OnCaptureEnd();
	
	/** 숨쉬기 효과를 재생한다. 이 함수는 숨쉬기 효과가 활성화되었을 때만 호출된다. */
	void PlayBreathEffects();

	/** 숨쉬기 효과 재생을 시작한다. */
	void StartBreathEffect(float Delay);

	/** 숨쉬기 효과 재생을 중지한다. */
	void StopBreathEffect();

	/** 공기 방울 효과를 생성한다. */
	void SpawnBreathBubbleEffect();

	/** 수중에서 이동 소리를 업데이트한다. 이 함수는 수중에서 이동 중일 때만 호출된다. */
	void UpdateMovementEffects(const float DeltaTime);

	/** 급격한 속도 변화가 있을 경우에 사운드를 재생한다. */
	void CheckVelocityChange(const float DeltaTime);

#pragma endregion

#pragma region Variable

private:
	/** AUnderwaterCharacter Cache */
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

	/** 숨쉬기 공기 방울이 생성될 소켓 위치 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	FName BreathSocketName;

	/** 대기 중에 재생할 숨쉬기 효과 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	ESFX IdleBreathSound;

	/** 움직일 때 재생될 숨쉬기 효과 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	ESFX MoveBreathSound;

	/** Sound to play when entering combat */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<USoundBase> CombatSound;

	/** 숨쉬기 효과를 생성할 Niagara 시스템. 이 시스템은 숨쉬기 효과를 시각적으로 표현한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	TObjectPtr<class UNiagaraSystem> BreathBubbleEffect;

	/** 숨쉬기 효과 재생 타이머 핸들. 숨쉬기 효과를 주기적으로 재생하는 데 사용된다. */
	FTimerHandle BreathEffectTimerHandle;

	/** 숨쉬기 효과 후에 재생할 공기 방울 효과 핸들. 숨을 내쉴 때 공기 방울 효과를 재생하는 데 사용된다. */
	FTimerHandle BreathBubbleEffectTimerHandle;

	/** 수중에서 이동 소리를 재생하고 있는지 여부. 이 값이 true일 때만 이동 소리가 재생된다. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|UnderwaterEffect", meta = (AllowPrivateAccess = "true"))
	uint8 bShouldPlayMovementEffect : 1;

	/** 수중에서 이동할 때 재생하는 사운드 ID */
	int32 MovementAudioId;

	/** Audio component for playing background sound when entering Combat. This component plays the CombatSound. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> CombatAudioComponent = nullptr;

	/** 수중에서 스프린트 할 때 재생하는 사운드 컴포넌트 */
	int32 SprintMovementAudioId;

	/** 수중에서 이동 소리 재생을 위한 사운드. 이 사운드는 MovementSoundThreshold보다 큰 속도로 이동할 때만 재생된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	ESFX MovementSound;

	/** 수중에서 달리기 이동 소리 재생을 위한 사운드. 이 사운드는 SprintMovementSound를 재생한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|UnderwaterEffect")
	ESFX SprintMovementSound;

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

	/** 소리 재생할 사운드 서브 시스템 Weak Pointer */
	TWeakObjectPtr<USoundSubsystem> SoundSubsystemWeakPtr;

	/** 움직일 때 생성되는 공기 방울 이펙트. 스프린트 시에는 더 많은 공기 방울이 생성된다. */
	UPROPERTY(EditAnywhere, Category = "Character|UnderwaterEffect")
	TObjectPtr<UNiagaraSystem> MoveBubbleParticleSystem;

	/** 움직일 때 생성되는 공기 방울 이펙트가 생성될 소켓 이름 */
	UPROPERTY(EditAnywhere, Category = "Character|UnderwaterEffect")
	FName MoveBubbleSocketName = TEXT("move_bubble_socket");

	/** 움직일 때 생성되는 공기 방울 이펙트 컴포넌트. 이 컴포넌트는 캐릭터의 메시에 부착되어 공기 방울을 생성한다. */
	UPROPERTY(Transient)
	TObjectPtr<class UNiagaraComponent> MoveBubbleParticleComponent;

	/** 움직일 때 공기 방울의 강도를 제어하는 파라미터 이름, 현재는 2개의 Bubble이 존재한다.
	 * Bubble Intensity를 키우면 큰 Bubble이 추가적으로 생성된다. */
	UPROPERTY(EditAnywhere, Category = "Character|UnderwaterEffect")
	FName MoveBubbleIntensityParameterName = TEXT("BigBubbleIntensity");

#pragma endregion

#pragma region Getter Setter



protected:
	/** 사운드 서브 시스템 접근 */
	USoundSubsystem* GetSoundSubsystem();

#pragma endregion
};
