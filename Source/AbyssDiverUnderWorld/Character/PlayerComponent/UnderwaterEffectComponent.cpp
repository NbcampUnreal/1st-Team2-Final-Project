// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterEffectComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

UUnderwaterEffectComponent::UUnderwaterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BreathInterval = 10.0f;
	BreathFirstDelay = 5.0f;
	BreathSocketName = TEXT("head_bubble_socket");

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
		OwnerCharacter->OnDamageTakenDelegate.AddDynamic(this, &UUnderwaterEffectComponent::OnDamageTaken);

		if (OwnerCharacter->IsLocallyControlled() && MovementSound)
		{
			MovementAudioComponent = UGameplayStatics::SpawnSoundAttached(
				MovementSound,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false,
				0.0f,
				1.0f,
				0.0f,
				nullptr,
				nullptr,
				false
			);
			MovementAudioComponent->SetAutoActivate(false);
			MovementAudioComponent->Stop();
			MovementAudioComponent->SetVolumeMultiplier(0.5f);

			SprintMovementAudioComponent = UGameplayStatics::SpawnSoundAttached(
				SprintMovementSound,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false,
				0.0f,
				1.0f,
				0.0f,
				nullptr,
				nullptr,
				false
			);
			SprintMovementAudioComponent->SetAutoActivate(false);
			SprintMovementAudioComponent->Stop();
			SprintMovementAudioComponent->SetVolumeMultiplier(0.5f);
		}
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
		GetWorld()->GetTimerManager().SetTimer(
			BreathEffectTimerHandle,
			this,
			&UUnderwaterEffectComponent::PlayBreathEffects,
			BreathInterval,
			true,
			BreathFirstDelay
		);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(BreathEffectTimerHandle);
		if (MovementAudioComponent && MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->Stop();
		}
		MoveTimeAccumulator = 0.0f;
	}
}

void UUnderwaterEffectComponent::OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState,
                                                           EEnvironmentState NewEnvironmentState)
{
	SetEnableEffect(NewEnvironmentState == EEnvironmentState::Underwater);
}

void UUnderwaterEffectComponent::OnDamageTaken(float DamageAmount, float CurrentHealth)
{
	// 피해를 입으면 피해 사운드가 재생되어야 한다.
	// 숨쉬기 효과를 초기화하고 버블 효과를 중지한다.
	GetWorld()->GetTimerManager().SetTimer(
			BreathEffectTimerHandle,
			this,
			&UUnderwaterEffectComponent::PlayBreathEffects,
			BreathInterval,
			true,
			BreathFirstDelay / 2.0f
	);
	GetWorld()->GetTimerManager().ClearTimer(BreathBubbleEffectTimerHandle);
}

void UUnderwaterEffectComponent::PlayBreathEffects()
{
	if (!bEnabled)
	{
		return;
	}

	USoundBase* BreathSoundToPlay = bShouldPlayMovementSound ? MoveBreathSound : IdleBreathSound;
	if (!BreathSoundToPlay)
	{
		return;
	}
	
	if (OwnerCharacter->IsLocallyControlled())
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

	const float Duration = BreathSoundToPlay->GetDuration();
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
		if (!MovementAudioComponent->IsPlaying()
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
		if (Speed > OwnerCharacter->GetSprintSpeed() - 50.0f
			&& SprintMovementSound
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
	
}
