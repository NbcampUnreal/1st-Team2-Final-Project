// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterEffectComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UUnderwaterEffectComponent::UUnderwaterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BreathInterval = 10.0f;
	BreathFirstDelay = 5.0f;
	// BreathBubbleLocationOffset = FVector(0.0f, 0.0f, 0.0f);

	MovementSoundThreshold = 300.0f;
	MoveRequireTime = 0.5f;
	MoveTimeAccumulator = 0.0f;
	MovementAudioComponent = nullptr;
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

		if (OwnerCharacter->IsLocallyControlled())
		{
			MovementAudioComponent = UGameplayStatics::SpawnSoundAttached(
				MovementSound,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false
			);
			MovementAudioComponent->SetAutoActivate(false);
			MovementAudioComponent->Stop();
			MovementAudioComponent->SetVolumeMultiplier(0.2f);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UUnderwaterFXComponent: Owner is not an AUnderwaterCharacter"));
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

	UpdateMovementEffects(DeltaTime);
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

void UUnderwaterEffectComponent::PlayBreathEffects()
{
	if (!bEnabled)
	{
		return;
	}

	if (BreathSound && OwnerCharacter->IsLocallyControlled())
	{
		// @ToDo : Sound Subsystem으로 변경
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			BreathSound,
			GetOwner()->GetActorLocation(),
			0.5f,
			1.0f,
			0.0f
		);
	}

	// @ToDo : Trail 효과로 변경
	// @ToDo : Bubble Spawn 시점을 숨을 내쉬고 이후로 변경
	if (BreathBubbleEffect)
	{
		UE_LOG(LogTemp,Display, TEXT("UUnderwaterEffectComponent::PlayBreathEffects - Spawning Breath Bubble Effect"));
		const FVector SpawnLocation = OwnerCharacter->GetMesh1P()->GetSocketLocation(TEXT("head_bubble_socket"));
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
	float Speed = OwnerCharacter->GetVelocity().Size();

	const bool bCurrentMoving = Speed > MovementSoundThreshold;
	if (bCurrentMoving)
	{
		MoveTimeAccumulator += DeltaTime;
	}
	else
	{
		MoveTimeAccumulator = 0.0f;
	}

	if (bCurrentMoving && MoveTimeAccumulator > MoveRequireTime)
	{
		if (!MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->Play();
		}
	}
}

