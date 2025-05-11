// Fill out your copyright notice in the Description page of Project Settings.


#include "OxygenComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"

UOxygenComponent::UOxygenComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	
}

void UOxygenComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UOxygenComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bOxygenSystemEnabled)
	{
		return;
	}
	
	if (GetOwnerRole() == ROLE_Authority && bShouldConsumeOxygen && OxygenLevel > 0)
	{
		ConsumeOxygen(DeltaTime);
	}
}

void UOxygenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOxygenComponent, MaxOxygenLevel);
	DOREPLIFETIME(UOxygenComponent, OxygenLevel);
}

void UOxygenComponent::OnRep_MaxOxygenLevel()
{
	OnOxygenLevelChanged.Broadcast(OxygenLevel, MaxOxygenLevel);
	K2_OnOxygenLevelChanged(OxygenLevel, MaxOxygenLevel);

	// MaxOxygenLevel을 변경하게 되면 OxygenLevel도 변경되어서 Replicate가 2번 발생한다.
	if (OxygenLevel > MaxOxygenLevel)
	{
		SetOxygenLevel(MaxOxygenLevel);
	}
	else
	{
		OnOxygenLevelChanged.Broadcast(OxygenLevel, MaxOxygenLevel);
		K2_OnOxygenLevelChanged(OxygenLevel, MaxOxygenLevel);
	}
}

void UOxygenComponent::OnRep_OxygenLevel()
{
	OnOxygenLevelChanged.Broadcast(OxygenLevel, MaxOxygenLevel);
	K2_OnOxygenLevelChanged(OxygenLevel, MaxOxygenLevel);
	
	if (OxygenLevel <= 0)
	{
		OnOxygenDepleted.Broadcast();
		K2_OnOxygenDepleted();
	}

	if (OldOxygenLevel <= 0 && OxygenLevel > 0)
	{
		OnOxygenRestored.Broadcast();
		K2_OnOxygenRestored();
	}

	OldOxygenLevel = OxygenLevel;
}

void UOxygenComponent::ConsumeOxygen(const float DeltaTime)
{
	SetOxygenLevel(OxygenLevel - OxygenConsumeRate * DeltaTime);
}

void UOxygenComponent::SetOxygenSystemEnabled(const bool bNewOxygenSystemEnabled)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	
	bOxygenSystemEnabled = bNewOxygenSystemEnabled;
}

void UOxygenComponent::SetShouldConsumeOxygen(const bool bNewShouldConsumeOxygen)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	
	bShouldConsumeOxygen = bNewShouldConsumeOxygen;
}

void UOxygenComponent::SetMaxOxygenLevel(float NewMaxOxygenLevel)
{
	if (GetOwnerRole() != ROLE_Authority || NewMaxOxygenLevel <= 0)
	{
		return;
	}

	MaxOxygenLevel = NewMaxOxygenLevel;
	if (OxygenLevel > MaxOxygenLevel)
	{
		SetOxygenLevel(MaxOxygenLevel);
	}
	else
	{
		// 현재는 MaxOxygenLevel이 변화하는 이벤트가 따로 있지는 않는다.
		// 공통적으로 OnOxygenLevelChanged를 호출한다.
		OnOxygenLevelChanged.Broadcast(OxygenLevel, MaxOxygenLevel);
		K2_OnOxygenLevelChanged(OxygenLevel, MaxOxygenLevel);
	}
}

EOXygenChangeResult UOxygenComponent::RefillOxygen(const float RefillAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return  EOXygenChangeResult::NotAuthority;
	}
	if (!IsOxygenSystemEnabled())
	{
		return EOXygenChangeResult::SystemDisabled;
	}
	if (OxygenLevel >= MaxOxygenLevel)
	{
		return EOXygenChangeResult::BlockedByLimit;
	}

	SetOxygenLevel(OxygenLevel + RefillAmount);
	
	return EOXygenChangeResult::Success;
}

void UOxygenComponent::SetOxygenLevel(const float NexOxygenLevel, const bool bForce = false)
{
	if (OxygenLevel == NexOxygenLevel && !bForce)
	{
		return;
	}

	float OldOxygenLevel = OxygenLevel;
	
	OxygenLevel  = FMath::Clamp(NexOxygenLevel, 0.0f, MaxOxygenLevel);
	OnOxygenLevelChanged.Broadcast(OxygenLevel, MaxOxygenLevel);
	K2_OnOxygenLevelChanged(OxygenLevel, MaxOxygenLevel);
	
	if (OxygenLevel <= 0)
	{
		OnOxygenDepleted.Broadcast();
		K2_OnOxygenDepleted();
	}

	if (OldOxygenLevel <= 0 && OxygenLevel > 0)
	{
		OnOxygenRestored.Broadcast();
		K2_OnOxygenRestored();
	}

	LOGN(TEXT("OxygenLevel : %f, MaxOxygenLevel : %f"), OxygenLevel, MaxOxygenLevel);
}
