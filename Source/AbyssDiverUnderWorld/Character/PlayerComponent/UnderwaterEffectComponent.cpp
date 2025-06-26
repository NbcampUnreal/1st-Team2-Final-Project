// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterEffectComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

UUnderwaterEffectComponent::UUnderwaterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BreathInterval = 10.0f;
	BreathFirstDelay = 5.0f;
	BreathSocketName = TEXT("head_bubble_socket");

	bShouldPlayMovementSound = false;
	MovementSoundThreshold = 300.0f;
	MoveRequireTime = 0.5f;
	MoveTimeAccumulator = 0.0f;
	
	MovementSoundFadeTime = 0.5f;
	MovementSoundFadeCurve = EAudioFaderCurve::Linear;
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

		// BugFix: Respawn 시에 Host에서 크래시나는 현상 수정
		// 항상 Audio Component를 생성하도록 변경
		// Respawn 상황에서 Begin Play 시점에서 Controller가 설정되기 전에 Begin Play가 호출된다.
		// 이 시점에서는 Controller가 없기 때문에 Locally Controlled 여부를 확인할 수 없다.
		MovementAudioComponent = UGameplayStatics::SpawnSoundAttached(
			MovementSound,
			GetOwner()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false,
			0.5f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			false
		);
		MovementAudioComponent->Stop();

		SprintMovementAudioComponent = UGameplayStatics::SpawnSoundAttached(
			SprintMovementSound,
			GetOwner()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false,
			0.5f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			false
		);
		SprintMovementAudioComponent->Stop();
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("UUnderwaterFXComponent: Owner is not an AUnderwaterCharacter"));
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

	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
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
		
		if (MovementAudioComponent && MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->FadeOut(MovementSoundFadeTime, 0.0f, MovementSoundFadeCurve);
		}
		if (SprintMovementAudioComponent && SprintMovementAudioComponent->IsPlaying())
		{
			SprintMovementAudioComponent->Stop();
		}
		MoveTimeAccumulator = 0.0f;
		bShouldPlayMovementSound = false;
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
	bShouldPlayMovementSound = false;
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

void UUnderwaterEffectComponent::PlayBreathEffects()
{
	if (!bEnabled)
	{
		return;
	}

	// @ToDo : Groggy 상태에서 다른 효과를 재생해야 할 수 있다.
	USoundBase* BreathSoundToPlay = bShouldPlayMovementSound ? MoveBreathSound : IdleBreathSound;
	
	if (OwnerCharacter->IsLocallyControlled() && BreathSoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			BreathSoundToPlay,
			GetOwner()->GetActorLocation(),
			1.0f,
			1.0f,
			0.0f
		);
	}

	// Breath Sound가 없으면 3초로 간주하고 버블 효과를 생성한다.
	const float Duration =  BreathSoundToPlay ? BreathSoundToPlay->GetDuration() : 3.0f;
	const float DelayToNextBubble = FMath::FRandRange(Duration * 0.5f, Duration * 0.6f);
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
	
	const bool bWasMoving = bShouldPlayMovementSound;
	bShouldPlayMovementSound = bCurrentMoving && MoveTimeAccumulator > MoveRequireTime;

	if (bShouldPlayMovementSound)
	{
		if (MovementAudioComponent
			&& !MovementAudioComponent->IsPlaying()
			&& MovementSound)
		{
			if (bWasMoving)
			{
				const float Duration = MovementSound->GetDuration();
				const float RandomStartTime = FMath::FRandRange(0.0f, Duration);
				MovementAudioComponent->Play(RandomStartTime);	
			}
			else
			{
				MovementAudioComponent->Play();
			}
		}
		
		// 플레이어가 직접 움직일 때 소리가 나야 한다.
		// 캐릭터가 Launch 되거나 밀릴 경우 소리가 나면 안 된다.
		if (Speed > OwnerCharacter->GetSprintSpeed() - 50.0f
			&& OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f
			&& SprintMovementSound
			&& SprintMovementAudioComponent
			&& !SprintMovementAudioComponent->IsPlaying())
		{
			SprintMovementAudioComponent->Play();
		}
	}
	else
	{
		if (MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->FadeOut(MovementSoundFadeTime, 0.0f, MovementSoundFadeCurve);
		}
	}
}

void UUnderwaterEffectComponent::CheckVelocityChange(const float DeltaTime)
{
	// @ToDo: 캐릭터가 급격한 속도 변화를 감지했을 때 소리 재생, 속도를 감지하거나 아니면 ABP에서 Notify를 받는다.
}
