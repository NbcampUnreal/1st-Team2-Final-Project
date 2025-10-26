// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/Components/MonsterSoundComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SoundSubsystem.h"
#include "Net/UnrealNetwork.h"


UMonsterSoundComponent::UMonsterSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PatrolLoopComponent = nullptr;
	ChaseLoopComponent = nullptr;
	FleeLoopComponent = nullptr;
}

void UMonsterSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	// 오디오 컴포넌트 준비해두고 꺼둠.
	if (PatrolLoopSound.IsValid())
	{
		PatrolLoopComponent = CreateLoopAudioComp(PatrolLoopSound.Get());
	}

	if (ChaseLoopSound.IsValid())
	{
		ChaseLoopComponent = CreateLoopAudioComp(ChaseLoopSound.Get());
	}

	if (FleeLoopSound.IsValid())
	{
		FleeLoopComponent = CreateLoopAudioComp(FleeLoopSound.Get());
	}
}

UAudioComponent* UMonsterSoundComponent::CreateLoopAudioComp(USoundBase* InSound)
{
	if (!GetOwner() || !InSound) return nullptr;

	UAudioComponent* Comp = NewObject<UAudioComponent>(GetOwner()); 
	Comp->bAutoActivate = false;
	Comp->bAutoDestroy = false;        
	Comp->bAllowSpatialization = true; // 3D Sound
	Comp->SetupAttachment(GetOwner()->GetRootComponent());
	Comp->RegisterComponent();
	Comp->SetSound(InSound);
	Comp->Stop();

	return Comp;
}

float UMonsterSoundComponent::GetTargetVolumeScale() const
{
	float OriginalVolume = 1.0f;
	float DesireVolume = OriginalVolume;

	if (SoundSubsystem.IsValid())
	{
		DesireVolume *= SoundSubsystem->GetSFXVolume();
		DesireVolume *= SoundSubsystem->GetMasterVolume();
	}
	else
	{
		// lazy fetch
		if (const_cast<UMonsterSoundComponent*>(this)->GetSoundSubsystem())
		{
			DesireVolume *= SoundSubsystem->GetSFXVolume();
			DesireVolume *= SoundSubsystem->GetMasterVolume();
		}
	}
	return DesireVolume;
}

void UMonsterSoundComponent::S_PlayPatrolLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	M_PlayPatrolLoopSound();
}

void UMonsterSoundComponent::M_PlayPatrolLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	if (PatrolLoopComponent.Get() && !PatrolLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = GetTargetVolumeScale();

		PatrolLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// PatrolLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayChaseLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	M_PlayChaseLoopSound();
}

void UMonsterSoundComponent::M_PlayChaseLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	if (ChaseLoopComponent.Get() && !ChaseLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = GetTargetVolumeScale();

		ChaseLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// ChaseLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayFleeLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	M_PlayFleeLoopSound();
}

void UMonsterSoundComponent::M_PlayFleeLoopSound_Implementation()
{
	if (FleeLoopComponent.Get() && !FleeLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = GetTargetVolumeScale();

		FleeLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// FleeLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayHitReactSound_Implementation()
{
	if (!IsValid(this)) return;

	M_PlayHitReactSound();
}

void UMonsterSoundComponent::M_PlayHitReactSound_Implementation()
{
	if (!IsValid(this)) return;

	if (HitReactSound.IsValid())
	{
		float Desired = 1.0f;
		float NewVolume = Desired * GetSoundSubsystem()->GetSFXVolume()* GetSoundSubsystem()->GetMasterVolume();
		UGameplayStatics::PlaySoundAtLocation(this, HitReactSound.Get(), GetOwner()->GetActorLocation(), NewVolume);
	}
}

void UMonsterSoundComponent::M_PlayDeathSound_Implementation()
{
	if (!IsValid(this)) return;

	if (DeathSound.Get())
	{
		float Desired = 1.0f;
		float NewVolume = Desired * GetSoundSubsystem()->GetSFXVolume() * GetSoundSubsystem()->GetMasterVolume();
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound.Get(), GetOwner()->GetActorLocation(), NewVolume);
	}
}

void UMonsterSoundComponent::S_StopAllLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	M_StopAllLoopSound();
}

void UMonsterSoundComponent::M_StopAllLoopSound_Implementation()
{
	if (!IsValid(this)) return;

	const float FadeOutDuration = 0.2f; // Time for sound to fade
	const float FadeOutVolume = 0.0f;   // Final Volume

	UE_LOG(LogTemp, Warning, TEXT("M_StopAllLoopSound_Implementation CALLED"));
	if (PatrolLoopComponent.IsValid() && PatrolLoopComponent->IsPlaying())
	{
		// PatrolLoopComponent->Stop();
		PatrolLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}

	if (ChaseLoopComponent.IsValid() && ChaseLoopComponent->IsPlaying())
	{
		// ChaseLoopComponent->Stop();
		ChaseLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}

	if (FleeLoopComponent.IsValid() && FleeLoopComponent->IsPlaying())
	{
		// FleeLoopComponent->Stop();
		FleeLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}
}

void UMonsterSoundComponent::S_RequestMainSoundDuck_Implementation(float DuckVolume, float DuckDuration, float RecoverDuration)
{
	if (!IsValid(this)) return;

	// Decrease volume (default DuckVolume : 0.2f)
	M_AdjustMainSoundVolume(DuckVolume, DuckDuration);

	// Always capture securely with WeakThis (against this destruction/GC)
	TWeakObjectPtr<UMonsterSoundComponent> WeakThis(this);

	float TargetVolume = GetTargetVolumeScale();

	FTimerHandle DuckTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DuckTimerHandle,
		[WeakThis, RecoverDuration, TargetVolume]()
		{
			if (!WeakThis.IsValid()) return;
			auto* SoundComp = WeakThis.Get();

			if ((IsValid(SoundComp->PatrolLoopComponent.Get()) && SoundComp->PatrolLoopComponent->IsPlaying()) ||
				(IsValid(SoundComp->ChaseLoopComponent.Get()) && SoundComp->ChaseLoopComponent->IsPlaying()) ||
				(IsValid(SoundComp->FleeLoopComponent.Get()) && SoundComp->FleeLoopComponent->IsPlaying()))
			{
				SoundComp->M_AdjustMainSoundVolume(TargetVolume, RecoverDuration);
			}
		},
		DuckDuration + 0.1f, false
	);
}

void UMonsterSoundComponent::M_AdjustMainSoundVolume_Implementation(float Volume, float FadeTime)
{
	if (!IsValid(this)) return;

	auto FadeComponent = [](UAudioComponent* Comp, float Volume, float Time)
	{
		if (Comp && Comp->IsPlaying())
		{
			Comp->AdjustVolume(Time, Volume);
		}
	};

	FadeComponent(PatrolLoopComponent.Get(), Volume, FadeTime);
	FadeComponent(ChaseLoopComponent.Get(), Volume, FadeTime);
	FadeComponent(FleeLoopComponent.Get(), Volume, FadeTime);
}

class USoundSubsystem* UMonsterSoundComponent::GetSoundSubsystem()
{
	if (!SoundSubsystem.IsValid())
	{
		if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
		{
			SoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SoundSubsystem is not valid and GameInstance is not found."));
		}
	}

	return SoundSubsystem.Get();
}