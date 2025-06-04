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
			MovementAudioComponent->SetVolumeMultiplier(0.2f);
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
		if (MovementAudioComponent && !MovementAudioComponent->IsPlaying())
		{
			MovementAudioComponent->Play();
		}
		else if (!IsValid(MovementAudioComponent))
		{
			UE_LOG(LogAbyssDiverCharacter, Error, TEXT("MovementAudioComponent is not valid"));
		}
	}
}

