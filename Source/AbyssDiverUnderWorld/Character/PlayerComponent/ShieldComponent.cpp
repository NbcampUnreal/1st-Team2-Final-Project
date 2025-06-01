// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldComponent.h"

#include "Net/UnrealNetwork.h"


UShieldComponent::UShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bCanGainShield = true;
	ShieldValue = 0.0f;
	OldShieldValue = ShieldValue;
}


// Called when the game starts
void UShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	OldShieldValue = ShieldValue;
}

void UShieldComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShieldComponent, ShieldValue);
}

float UShieldComponent::TakeDamage(const float DamageAmount)
{
	if (GetOwnerRole() != ROLE_Authority || DamageAmount <= 0.0f || ShieldValue <= 0.0f)
	{
		return 0.0f;
	}

	const float PostShieldValue = ShieldValue - DamageAmount;
	SetShieldValue(ShieldValue - DamageAmount);
	// 남은 데미지가 0보다 작으면 남은 데미지를 반환
	return PostShieldValue > 0.0f ? 0.0f : -PostShieldValue;
}

void UShieldComponent::GainShield(const float GainAmount)
{
	if (GetOwnerRole() != ROLE_Authority || GainAmount <= 0.0f || !bCanGainShield)
	{
		return;
	}

	SetShieldValue(ShieldValue + GainAmount);
}

void UShieldComponent::OnRep_ShieldValueChanged()
{
	SetShieldValue(ShieldValue, true);
}

void UShieldComponent::SetShieldValue(const float NewValue, const bool bAlwaysUpdate)
{
	if (ShieldValue == NewValue && !bAlwaysUpdate)
	{
		return;
	}

	OldShieldValue = ShieldValue;

	ShieldValue = NewValue > 0.0f ? NewValue : 0.0f;
	OnShieldValueChangedDelegate.Broadcast(ShieldValue);
	K2_OnShieldValueChanged(ShieldValue);

	// bAlwaysUpdate가 있기 때문에 OldShieldValue를 확인해야 한다.
	if (ShieldValue <= 0.0f && OldShieldValue > 0.0f) 
	{
		OnShieldBrokenDelegate.Broadcast();
		K2_OnShiledBroken();
	}
	if (ShieldValue > 0.0f && OldShieldValue <= 0.0f)
	{
		OnShieldGainedDelegate.Broadcast();
		K2_OnShieldGained();
	}
}


