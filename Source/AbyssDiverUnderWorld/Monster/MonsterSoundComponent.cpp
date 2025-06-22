// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterSoundComponent.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"


UMonsterSoundComponent::UMonsterSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PatrolLoopComponent = nullptr;
	ChaseLoopComponent = nullptr;
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
		PatrolLoopComponent->Play();
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
		ChaseLoopComponent->Play();
	}
}

void UMonsterSoundComponent::S_StopAllLoopSound_Implementation()
{
	M_StopAllLoopSound();
}

void UMonsterSoundComponent::M_StopAllLoopSound_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MulticastPlayChaseLoop CALLED"));
	if (PatrolLoopComponent && PatrolLoopComponent->IsPlaying())
	{
		PatrolLoopComponent->Stop();
	}

	if (ChaseLoopComponent && ChaseLoopComponent->IsPlaying())
	{
		ChaseLoopComponent->Stop();
	}
}

