// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterEffectComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Subsystems/SoundSubsystem.h"

UUnderwaterEffectComponent::UUnderwaterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BreathInterval = 10.0f;
	BreathFirstDelay = 5.0f;
	BreathSocketName = TEXT("head_bubble_socket");

	bShouldPlayMovementEffect = false;
	MovementSoundThreshold = 300.0f;
	MoveRequireTime = 0.5f;
	MoveTimeAccumulator = 0.0f;
	
	MovementSoundFadeTime = 0.5f;
	MovementSoundFadeCurve = EAudioFaderCurve::Linear;

	IdleBreathSound = ESFX::IdleBreath;
	MoveBreathSound = ESFX::MoveBreath;
	MovementSound = ESFX::UnderwaterMovement;
	SprintMovementSound = ESFX::UnderwaterSprint;

	MovementAudioId = INDEX_NONE;
	SprintMovementAudioId = INDEX_NONE;
}

void UUnderwaterEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetOwner()))
	{
		OwnerCharacter = UnderwaterCharacter;

		const EEnvironmentState CurrentEnvironmentState = OwnerCharacter->GetEnvironmentState();
		SetEnableEffect(CurrentEnvironmentState == EEnvironmentState::Underwater);
		
		OwnerCharacter->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnEnvironmentStateChanged);
		OwnerCharacter->OnDeathDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnDeath);
		OwnerCharacter->OnDamageTakenDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnDamageTaken);
		OwnerCharacter->OnKnockbackDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnKnockback);
		OwnerCharacter->OnKnockbackEndDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnKnockbackEnd);
		OwnerCharacter->OnCaptureStartDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnCaptureStart);
		OwnerCharacter->OnCaptureEndDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnCaptureEnd);
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("UUnderwaterFXComponent: Owner is not an AUnderwaterCharacter"));
	}

	if (MoveBubbleParticleSystem && OwnerCharacter)
	{
		MoveBubbleParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			MoveBubbleParticleSystem,
			OwnerCharacter->GetMesh(),
			MoveBubbleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false,
			false
		);
		UE_LOG(LogAbyssDiverCharacter, Display, TEXT("UUnderwaterFXComponent: Spawning MoveBubbleParticleSystem on %s"), *OwnerCharacter->GetName());
	}
}

void UUnderwaterEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// @ToDo: 추후에 UnderwaterEffectComponent가 활성화되었을 때만 Tick하도록 변경
	if (!bEnabled)
	{
		return;
	}

	if (OwnerCharacter)
	{
		UpdateMovementEffects(DeltaTime);
	}
}

void UUnderwaterEffectComponent::SetEnableEffect(bool bNewEnabled)
{
	if (bEnabled == bNewEnabled)
	{
		return;
	}

	bEnabled = bNewEnabled;
	
	if (bEnabled)
	{
		StartBreathEffect(BreathFirstDelay);
	}
	else
	{
		StopBreathEffect();

		// 수중 이동 소리 중지
		USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
		if (SoundSubsystem && SoundSubsystem->IsPlaying(MovementAudioId))
		{
			SoundSubsystem->StopAudio(MovementSoundFadeTime, true, MovementSoundFadeTime, 0.0f, MovementSoundFadeCurve);
		}
		if (SoundSubsystem && SoundSubsystem->IsPlaying(SprintMovementAudioId))
		{
			SoundSubsystem->StopAudio(SprintMovementAudioId);
		}
		MoveTimeAccumulator = 0.0f;
		bShouldPlayMovementEffect = false;

		if (MoveBubbleParticleComponent && MoveBubbleParticleComponent->IsActive())
		{
			MoveBubbleParticleComponent->Deactivate();
		}
	}
}

void UUnderwaterEffectComponent::OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState,
                                                           EEnvironmentState NewEnvironmentState)
{
	SetEnableEffect(NewEnvironmentState == EEnvironmentState::Underwater);
}

void UUnderwaterEffectComponent::OnDeath()
{
	StopBreathEffect();
}

void UUnderwaterEffectComponent::StartBreathEffect(float Delay)
{
	GetWorld()->GetTimerManager().SetTimer(
		BreathEffectTimerHandle,
		this,
		&UUnderwaterEffectComponent::PlayBreathEffects,
		BreathInterval,
		true,
		Delay
	);
	GetWorld()->GetTimerManager().ClearTimer(BreathBubbleEffectTimerHandle);
}

void UUnderwaterEffectComponent::StopBreathEffect()
{
	GetWorld()->GetTimerManager().ClearTimer(BreathEffectTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BreathBubbleEffectTimerHandle);
}

void UUnderwaterEffectComponent::StartCombatEffect()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PC || !PC->IsLocalController()) return;

	// Don't play CombatSound if already playing
	if (CombatAudioComponent && CombatAudioComponent->IsPlaying()) return;
	
	if (CombatSound && !CombatAudioComponent)
	{
		CombatAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), CombatSound);
		if (CombatAudioComponent)
		{
			CombatAudioComponent->FadeIn(1.0f, 1.0f, 0.0f); // 1 second of FadeIn
			CombatAudioComponent->bAutoDestroy = true; // Auto-destroy after FadeOut (by default)
			CombatAudioComponent->SetVolumeMultiplier(1.0f); // Startup volume
		}
	}

}

void UUnderwaterEffectComponent::StopCombatEffect()
{
	if (CombatAudioComponent)
	{
		CombatAudioComponent->FadeOut(1.5f, 0.0f); //  Decrease naturally Sound for 1.5 seconds
		CombatAudioComponent = nullptr;
	}
}

void UUnderwaterEffectComponent::OnDamageTaken(float DamageAmount, float CurrentHealth)
{
	// 피해를 입으면 피해 사운드가 재생되어야 한다.
	// 숨쉬기 효과를 초기화하고 버블 효과를 중지한다.
	bShouldPlayMovementEffect = false;
	StartBreathEffect(BreathFirstDelay * 0.5f);
}

void UUnderwaterEffectComponent::OnKnockback(FVector KnockbackVelocity)
{
	StopBreathEffect();
}

void UUnderwaterEffectComponent::OnKnockbackEnd()
{
	StartBreathEffect(BreathFirstDelay * 0.5f);
}

void UUnderwaterEffectComponent::OnCaptureStart()
{
	StopBreathEffect();
}

void UUnderwaterEffectComponent::OnCaptureEnd()
{
	StartBreathEffect(BreathFirstDelay * 0.5f);
}

void UUnderwaterEffectComponent::PlayBreathEffects()
{
	if (!bEnabled)
	{
		return;
	}

	// @ToDo : Groggy 상태에서 다른 효과를 재생해야 할 수 있다.
	ESFX BreathSoundToPlay = bShouldPlayMovementEffect ? MoveBreathSound : IdleBreathSound;

	float AudioDuration = 3.0f;
	if (OwnerCharacter->IsLocallyControlled())
	{
		if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
		{
			const int32 AudioId = SoundSubsystem->Play2D(BreathSoundToPlay);
			float PlayedBreathDuration = SoundSubsystem->GetAudioTotalDuration(AudioId);
			AudioDuration = PlayedBreathDuration > 0.0f ? PlayedBreathDuration : AudioDuration;
		}
	}

	const float DelayToNextBubble = FMath::FRandRange(AudioDuration * 0.5f, AudioDuration * 0.6f);
	GetWorld()->GetTimerManager().SetTimer(
		BreathBubbleEffectTimerHandle,
		this,
		&UUnderwaterEffectComponent::SpawnBreathBubbleEffect,
		DelayToNextBubble,
		false
	);
}

void UUnderwaterEffectComponent::SpawnBreathBubbleEffect()
{
	// @ToDo : Trail 효과로 변경
	if (BreathBubbleEffect)
	{
		const FVector SpawnLocation = OwnerCharacter->GetMesh1P()->GetSocketLocation(BreathSocketName);
		const FRotator SpawnRotation = OwnerCharacter->GetActorRotation();

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BreathBubbleEffect,
			SpawnLocation,
			SpawnRotation,
			FVector::OneVector,
			true
		);
	}
}

void UUnderwaterEffectComponent::UpdateMovementEffects(float DeltaTime)
{
	const float Speed = OwnerCharacter->GetVelocity().Size();

	const bool bCurrentMoving = Speed > MovementSoundThreshold;
	MoveTimeAccumulator = bCurrentMoving ? MoveTimeAccumulator + DeltaTime : 0.0f;
	
	bShouldPlayMovementEffect = bCurrentMoving && MoveTimeAccumulator > MoveRequireTime;

	USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (!IsValid(SoundSubsystem))
	{
		return;
	}
	
	if (bShouldPlayMovementEffect)
	{
		// 플레이어 이동 소리 재생
		if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && !SoundSubsystem->IsPlaying(MovementAudioId))
		{
			MovementAudioId = SoundSubsystem->Play2D(MovementSound, 0.5f);
		}
		if (MoveBubbleParticleComponent && !MoveBubbleParticleComponent->IsActive())
		{
			MoveBubbleParticleComponent->Activate();
		}
		if (MoveBubbleParticleComponent && MoveBubbleParticleComponent->IsActive())
		{
			MoveBubbleParticleComponent->SetFloatParameter(MoveBubbleIntensityParameterName, 0.0f);
		}
		
		// 플레이어가 직접 움직일 때 소리가 나야 한다.
		// 캐릭터가 Launch 되거나 밀릴 경우 소리가 나면 안 된다.
		if (Speed > OwnerCharacter->GetSprintSpeed() - 50.0f
			&& OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f)
		{
			if (OwnerCharacter && OwnerCharacter->IsLocallyControlled()
				&& !SoundSubsystem->IsPlaying(SprintMovementAudioId))
			{
				SprintMovementAudioId = SoundSubsystem->Play2D(SprintMovementSound, 0.5f);
			}
			// 스프린트 중일 때 버블 강도 증가
			if (MoveBubbleParticleComponent)
			{
				MoveBubbleParticleComponent->SetFloatParameter(MoveBubbleIntensityParameterName, 1.0f);
			}
		}
	}
	else
	{
		if (OwnerCharacter && OwnerCharacter->IsLocallyControlled()
			&& SoundSubsystem->IsPlaying(MovementAudioId))
		{
			SoundSubsystem->StopAudio(MovementAudioId, true, MovementSoundFadeTime, 0.0f, MovementSoundFadeCurve);
		}
		if (MoveBubbleParticleComponent && MoveBubbleParticleComponent->IsActive())
		{
			MoveBubbleParticleComponent->Deactivate();
		}
	}
}

void UUnderwaterEffectComponent::CheckVelocityChange(const float DeltaTime)
{
	// @ToDo: 캐릭터가 급격한 속도 변화를 감지했을 때 소리 재생, 속도를 감지하거나 아니면 ABP에서 Notify를 받는다.
}

USoundSubsystem* UUnderwaterEffectComponent::GetSoundSubsystem()
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			SoundSubsystemWeakPtr = World->GetGameInstance()->GetSubsystem<USoundSubsystem>();
		}
	}

	return SoundSubsystemWeakPtr.Get();
}
