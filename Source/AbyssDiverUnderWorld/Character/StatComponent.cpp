// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"


UStatComponent::UStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	MoveSpeed = 400.0f;
	AttackPower = 10;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
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
}

