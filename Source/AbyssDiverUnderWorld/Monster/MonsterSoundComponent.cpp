// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterSoundComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
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

	if (PatrolLoopSound)
	{
		PatrolLoopComponent = NewObject<UAudioComponent>(this);
		PatrolLoopComponent->bAutoActivate = false;
		PatrolLoopComponent->bAutoDestroy = false;
		PatrolLoopComponent->SetupAttachment(GetOwner()->GetRootComponent());
		PatrolLoopComponent->bAllowSpatialization = true;
		PatrolLoopComponent->RegisterComponent();
		PatrolLoopComponent->Stop();
		PatrolLoopComponent->SetSound(PatrolLoopSound);
	}
	
	if (ChaseLoopSound)
	{
		ChaseLoopComponent = NewObject<UAudioComponent>(this);
		ChaseLoopComponent->bAutoActivate = false;
		ChaseLoopComponent->bAutoDestroy = false;
		ChaseLoopComponent->SetupAttachment(GetOwner()->GetRootComponent());
		ChaseLoopComponent->bAllowSpatialization = true;
		ChaseLoopComponent->RegisterComponent();
		ChaseLoopComponent->SetSound(nullptr);
		ChaseLoopComponent->Stop();

		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (ChaseLoopComponent)
			{
				ChaseLoopComponent->SetSound(ChaseLoopSound);
				ChaseLoopComponent->Stop();
			}
		});

		if (FleeLoopSound)
		{
			FleeLoopComponent = NewObject<UAudioComponent>(this);
			FleeLoopComponent->bAutoActivate = false;
			FleeLoopComponent->bAutoDestroy = false;
			FleeLoopComponent->SetupAttachment(GetOwner()->GetRootComponent());
			FleeLoopComponent->bAllowSpatialization = true;
			FleeLoopComponent->RegisterComponent();
			FleeLoopComponent->SetSound(nullptr);
			FleeLoopComponent->Stop();

			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				if (FleeLoopComponent)
				{
					FleeLoopComponent->SetSound(FleeLoopSound);
					FleeLoopComponent->Stop();
				}
			});
		}
	}
}

void UMonsterSoundComponent::S_PlayPatrolLoopSound_Implementation()
{
	M_PlayPatrolLoopSound();
}

void UMonsterSoundComponent::M_PlayPatrolLoopSound_Implementation()
{
	if (PatrolLoopComponent && !PatrolLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = 1.0f;

		PatrolLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// PatrolLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayChaseLoopSound_Implementation()
{
	M_PlayChaseLoopSound();
}

void UMonsterSoundComponent::M_PlayChaseLoopSound_Implementation()
{
	if (ChaseLoopComponent && !ChaseLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = 1.0f;

		ChaseLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// ChaseLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayFleeLoopSound_Implementation()
{
	M_PlayFleeLoopSound();
}

void UMonsterSoundComponent::M_PlayFleeLoopSound_Implementation()
{
	if (FleeLoopComponent && !FleeLoopComponent->IsPlaying())
	{
		const float FadeInDuration = 1.0f;
		const float TargetVolume = 1.0f;

		FleeLoopComponent->FadeIn(FadeInDuration, TargetVolume);
		// FleeLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_PlayHitReactSound_Implementation()
{
	M_PlayHitReactSound();
}

void UMonsterSoundComponent::M_PlayHitReactSound_Implementation()
{
	if (HitReactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitReactSound, GetOwner()->GetActorLocation());
	}
}

void UMonsterSoundComponent::S_StopAllLoopSound_Implementation()
{
	M_StopAllLoopSound();
}

void UMonsterSoundComponent::M_StopAllLoopSound_Implementation()
{
	const float FadeOutDuration = 0.2f; // Time for sound to fade
	const float FadeOutVolume = 0.0f;   // Final Volume

	UE_LOG(LogTemp, Warning, TEXT("MulticastPlayChaseLoop CALLED"));
	if (PatrolLoopComponent && PatrolLoopComponent->IsPlaying())
	{
		// PatrolLoopComponent->Stop();
		PatrolLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}

	if (ChaseLoopComponent && ChaseLoopComponent->IsPlaying())
	{
		// ChaseLoopComponent->Stop();
		ChaseLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}

	if (FleeLoopComponent && FleeLoopComponent->IsPlaying())
	{
		// FleeLoopComponent->Stop();
		FleeLoopComponent->FadeOut(FadeOutDuration, FadeOutVolume);
	}
}

void UMonsterSoundComponent::S_RequestMainSoundDuck_Implementation(float DuckVolume, float DuckDuration, float RecoverDuration)
{
	// Decrease volume (default DuckVolume : 0.2f)
	M_AdjustMainSoundVolume(DuckVolume, DuckDuration);

	FTimerHandle DuckTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DuckTimerHandle, [this, RecoverDuration]()
	{
		// Recover volume (1.0f)
		M_AdjustMainSoundVolume(1.0f, RecoverDuration);
	}, DuckDuration + 0.1f, false);
}

void UMonsterSoundComponent::M_AdjustMainSoundVolume_Implementation(float Volume, float FadeTime)
{
	auto FadeComponent = [](UAudioComponent* Comp, float Volume, float Time)
	{
		if (Comp && Comp->IsPlaying())
		{
			Comp->AdjustVolume(Time, Volume);
		}
	};

	FadeComponent(PatrolLoopComponent, Volume, FadeTime);
	FadeComponent(ChaseLoopComponent, Volume, FadeTime);
	FadeComponent(FleeLoopComponent, Volume, FadeTime);
}

