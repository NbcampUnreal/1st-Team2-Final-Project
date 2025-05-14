// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"

#include "Net/UnrealNetwork.h"


UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	MoveSpeed = 400.0f;
	AttackPower = 10;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatComponent, MaxHealth);
	DOREPLIFETIME(UStatComponent, CurrentHealth);
}

void UStatComponent::Initialize(int32 InitMaxHealth, int32 InitCurrentHealth, float InitMoveSpeed,
                                int32 InitAttackPower)
{
	MaxHealth = InitMaxHealth;
	CurrentHealth = InitCurrentHealth;
	MoveSpeed = InitMoveSpeed;
	AttackPower = InitAttackPower;
}

void UStatComponent::TakeDamage(const float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, MaxHealth);

	OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);
}

void UStatComponent::OnRep_MaxHealth()
{
	OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);
}

void UStatComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);
}

