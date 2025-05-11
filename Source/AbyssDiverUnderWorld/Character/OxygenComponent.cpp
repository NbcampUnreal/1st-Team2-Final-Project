// Fill out your copyright notice in the Description page of Project Settings.


#include "OxygenComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"

UOxygenComponent::UOxygenComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bOxygenSystemEnabled = true;
	bShouldConsumeOxygen = true;
	
	OxygenState.MaxOxygenLevel = 600.0f;
	OxygenState.OxygenLevel = 600.0f;

	OxygenConsumeRate = 1.0f;
}

void UOxygenComponent::BeginPlay()
{
	Super::BeginPlay();

	OldOxygenLevel = OxygenState.OxygenLevel;
}


void UOxygenComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bOxygenSystemEnabled)
	{
		return;
	}
	
	if (GetOwnerRole() == ROLE_Authority && bShouldConsumeOxygen && GetOxygenLevel() > 0)
	{
		ConsumeOxygen(DeltaTime);
	}
}

void UOxygenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOxygenComponent, OxygenState);
}

void UOxygenComponent::OnRep_OxygenStateChanged()
{
	// MaxOxygenLevel, OxygenLevel이 한 번에 변경되어서 Replicate 되므로 따로 값을 수정할 필요는 없다.
	OnOxygenLevelChanged.Broadcast(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);
	K2_OnOxygenLevelChanged(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);

	if (OxygenState.OxygenLevel <= 0)
	{
		OnOxygenDepleted.Broadcast();
		K2_OnOxygenDepleted();
	}

	if (OldOxygenLevel <= 0 && OxygenState.OxygenLevel > 0)
	{
		OnOxygenRestored.Broadcast();
		K2_OnOxygenRestored();
	}
	
	OldOxygenLevel = OxygenState.OxygenLevel;
}

void UOxygenComponent::ConsumeOxygen(const float DeltaTime)
{
	SetOxygenLevel(OxygenState.OxygenLevel - OxygenConsumeRate * DeltaTime);
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

	OxygenState.MaxOxygenLevel = NewMaxOxygenLevel;
	if (OxygenState.OxygenLevel > OxygenState.MaxOxygenLevel)
	{
		SetOxygenLevel(OxygenState.MaxOxygenLevel);
	}
	else
	{
		// 현재는 MaxOxygenLevel이 변화하는 이벤트가 따로 있지는 않는다.
		// 공통적으로 OnOxygenLevelChanged를 호출한다.
		OnOxygenLevelChanged.Broadcast(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);
		K2_OnOxygenLevelChanged(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);
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
	if (OxygenState.OxygenLevel >= OxygenState.MaxOxygenLevel)
	{
		return EOXygenChangeResult::BlockedByLimit;
	}

	SetOxygenLevel(OxygenState.OxygenLevel + RefillAmount);
	
	return EOXygenChangeResult::Success;
}

void UOxygenComponent::SetOxygenLevel(const float NextOxygenLevel, const bool bForce)
{
	if (OxygenState.OxygenLevel == NextOxygenLevel && !bForce)
	{
		return;
	}

	OldOxygenLevel = OxygenState.OxygenLevel;
	
	OxygenState.OxygenLevel  = FMath::Clamp(NextOxygenLevel, 0.0f, OxygenState.MaxOxygenLevel);
	OnOxygenLevelChanged.Broadcast(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);
	K2_OnOxygenLevelChanged(OxygenState.OxygenLevel, OxygenState.MaxOxygenLevel);
	
	if (OxygenState.OxygenLevel <= 0)
	{
		OnOxygenDepleted.Broadcast();
		K2_OnOxygenDepleted();
	}

	if (OldOxygenLevel <= 0 && OxygenState.OxygenLevel > 0)
	{
		OnOxygenRestored.Broadcast();
		K2_OnOxygenRestored();
	}
}
