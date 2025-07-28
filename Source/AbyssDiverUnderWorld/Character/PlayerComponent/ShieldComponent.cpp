// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Net/UnrealNetwork.h"


UShieldComponent::UShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	bCanGainShield = true;
	ShieldValue = 0.0f;
	OldShieldValue = ShieldValue;
	MaxShieldValue = 1000.0f; // 기본 최대 실드 값
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

FShieldAbsorbResult UShieldComponent::AbsorbDamage(const float DamageAmount)
{
	FShieldAbsorbResult Result;
	
	if (GetOwnerRole() != ROLE_Authority || DamageAmount <= 0.0f || ShieldValue <= 0.0f)
	{
		Result.RemainingDamage = DamageAmount;
		return Result;
	}

	
	Result.AbsorbedDamage = FMath::Min(ShieldValue, DamageAmount);
	SetShieldValue(ShieldValue - Result.AbsorbedDamage);
	Result.RemainingDamage = DamageAmount - Result.AbsorbedDamage;
	// UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Shield Absorb Damage : %f, Remaining Damage : %f"), Result.AbsorbedDamage, Result.RemainingDamage);
	return Result;
}

void UShieldComponent::GainShield(const float GainAmount)
{
	if (GetOwnerRole() != ROLE_Authority || GainAmount <= 0.0f || !bCanGainShield || IsShieldFull())
	{
		return;
	}

	SetShieldValue(ShieldValue + GainAmount);
}

void UShieldComponent::OnRep_ShieldValueChanged()
{
	OnShieldValueChangedDelegate.Broadcast(OldShieldValue, ShieldValue);
	K2_OnShieldValueChanged(ShieldValue);

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

	OldShieldValue = ShieldValue;
}

void UShieldComponent::SetShieldValue(const float NewValue, const bool bAlwaysUpdate)
{
	if (ShieldValue == NewValue && !bAlwaysUpdate)
	{
		return;
	}

	OldShieldValue = ShieldValue;

	ShieldValue = FMath::Clamp(NewValue, 0.0f, MaxShieldValue);
	OnShieldValueChangedDelegate.Broadcast(OldShieldValue, ShieldValue);
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


