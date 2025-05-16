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

	HealthUpdateInterval = 0.05f;
	HealthRegenRate = 0.0f;
	HealthAccumulator = 0.0f;
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
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, MaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UStatComponent::OnRep_MaxHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UStatComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UStatComponent::RegenHealth()
{
	float RegenAmount = HealthRegenRate * HealthUpdateInterval;
	HealthAccumulator += RegenAmount;
	int32 HealthToAdd = FMath::FloorToInt(HealthAccumulator);

	// 양수일 경우
	// HealthAccumulator == 1 + 0.5라 가정하면
	// HealthToAdd == 1
	// HealthAccumulator -= HealthToAdd => HealthAccumulator : 0.5
	// CurrentHealth == 100 + 1 = 101

	// 음수일 경우
	// HealthAccumulator == -1 - 0.5라 가정하면
	// HealthToAdd == -1
	// HealthAccumulator -= HealthToAdd => HealthAccumulator : -0.5
	// CurrentHealth == 100 - 1 = 99
	
	if (HealthToAdd != 0)
	{
		HealthAccumulator -= HealthToAdd;
		float OldHealth = CurrentHealth;
		float NewHealth = FMath::Clamp(CurrentHealth + HealthToAdd, 0, MaxHealth);
		if (OldHealth != NewHealth)
		{
			CurrentHealth = NewHealth;
			OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
		}
	}
}

void UStatComponent::StopHealthRegen()
{
	HealthRegenRate = 0.0f;
	HealthAccumulator = 0.0f;
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
}

bool UStatComponent::IsHealthRegenActive() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(HealthRegenTimerHandle);
}

void UStatComponent::SetHealthRegenRate(float NewHealthRegenRate)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	HealthRegenRate = NewHealthRegenRate;

	if (FMath::IsNearlyZero(NewHealthRegenRate))
	{
		StopHealthRegen();
		return;
	}

	if (!IsHealthRegenActive())
	{
		GetWorld()->GetTimerManager().SetTimer(
			HealthRegenTimerHandle,
			this,
			&UStatComponent::RegenHealth,
			HealthUpdateInterval,
			true
		);
	}
}

void UStatComponent::AddHealthRegenRate(const float AddHealthRegenRate)
{
	SetHealthRegenRate(HealthRegenRate + AddHealthRegenRate);
}

