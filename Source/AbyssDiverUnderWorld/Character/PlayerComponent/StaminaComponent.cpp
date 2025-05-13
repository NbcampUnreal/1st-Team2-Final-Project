// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	StaminaUpdateInterval = 0.02f;
	bIsSprinting = false;

	// Stamina는 Oxygen에 영향을 받는다.
	// Oxygen도 600에서 시작하므로 값을 일치한다.
	// Oxygen 600은 600초이지만 Sprint는 6초 정도의 시간으로 움직인다.
	StaminaStatus.MaxStamina = 600.f;
	StaminaStatus.Stamina = StaminaStatus.MaxStamina;
	SprintConsumeRate = 100.0f;
	SprintRegenRate = 100.0f;
	StaminaRegenDelay = 3.0f;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaminaComponent, bIsSprinting);
	DOREPLIFETIME(UStaminaComponent, StaminaStatus);
}

void UStaminaComponent::RequestStartSprint()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		S_StartSprint();
	}
	else
	{
		StartSprint();
	}
}

void UStaminaComponent::RequestStopSprint()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		S_StopSprint();
	}
	else
	{
		StopSprint();
	}
}

void UStaminaComponent::StartSprint()
{
	if (!CanSprint() || bIsSprinting)
	{
		return;
	}

	bIsSprinting = true;
	OnSprintStateChanged.Broadcast(bIsSprinting);

	// Stamina 회복 중이거나 딜레이 중이었으면 취소가 된다.
	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimeHandle,
		this,
		&UStaminaComponent::ConsumeStamina,
		StaminaUpdateInterval,
		true
	);
}

void UStaminaComponent::StopSprint()
{
	// Server에서 Stop이 먼저 되고 Client에서 Stop RPC가 오게 되면
	// 이미 처리되었으므로 무시한다.
	if (!bIsSprinting)
	{
		return;
	}
	
	bIsSprinting = false;
	OnSprintStateChanged.Broadcast(bIsSprinting);

	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimeHandle,
		this,
		&UStaminaComponent::StartRegenerateStamina,
		StaminaRegenDelay,
		false
	);
}

void UStaminaComponent::OnRep_StaminaStatusChanged()
{
	OnStaminaChanged.Broadcast(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
	K2_OnStaminaChanged(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
}

void UStaminaComponent::OnRep_IsSprintingChanged()
{
	OnSprintStateChanged.Broadcast(bIsSprinting);
}

void UStaminaComponent::S_StartSprint_Implementation()
{
	StartSprint();
}

void UStaminaComponent::S_StopSprint_Implementation()
{
	StopSprint();
}

void UStaminaComponent::ConsumeStamina()
{
	const float NewStamina = StaminaStatus.Stamina - SprintConsumeRate * StaminaUpdateInterval;
	SetStamina(NewStamina);

	if (bIsSprinting && StaminaStatus.Stamina <= 0.0f)
	{
		StopSprint();
	}
}

void UStaminaComponent::RegenerateStamina()
{
	const float NewStamina = StaminaStatus.Stamina + SprintRegenRate * StaminaUpdateInterval;
	SetStamina(NewStamina);

	if (StaminaStatus.Stamina >= StaminaStatus.MaxStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaTimeHandle);
	}
}

void UStaminaComponent::StartRegenerateStamina()
{
	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimeHandle,
		this,
		&UStaminaComponent::RegenerateStamina,
		StaminaUpdateInterval,
		true
	);
}

void UStaminaComponent::InitStamina(float MaxStamina, float Stamina)
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		return;
	}

	StaminaStatus.MaxStamina = MaxStamina;
	SetStamina(Stamina);
}

void UStaminaComponent::SetMaxStamina(const float NewMaxStamina)
{
	if (GetOwnerRole() == ROLE_AutonomousProxy || NewMaxStamina <= 0)
	{
		return;
	}

	StaminaStatus.MaxStamina = NewMaxStamina;
	if (StaminaStatus.Stamina > StaminaStatus.MaxStamina)
	{
		SetStamina(StaminaStatus.MaxStamina);
	}
	else
	{
		OnStaminaChanged.Broadcast(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
		K2_OnStaminaChanged(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
	}
}

void UStaminaComponent::SetStamina(const float NewStamina)
{
	StaminaStatus.Stamina = FMath::Clamp(NewStamina, 0.0f, StaminaStatus.MaxStamina);
	OnStaminaChanged.Broadcast(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
	K2_OnStaminaChanged(StaminaStatus.MaxStamina, StaminaStatus.Stamina);
}

