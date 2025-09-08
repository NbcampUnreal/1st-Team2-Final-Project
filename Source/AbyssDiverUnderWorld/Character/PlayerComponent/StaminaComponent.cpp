// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

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

	OwnerCharacter = Cast<AUnderwaterCharacter>(GetOwner());
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaminaComponent, bIsSprinting);
	DOREPLIFETIME(UStaminaComponent, StaminaStatus);
}

void UStaminaComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (bIsSprinting && IsCharacterSprinting())
	{
		ConsumeStamina(DeltaTime);
		TimeSinceLastSprint = 0.0f;
	}
	else
	{
		TimeSinceLastSprint += DeltaTime;

		if (TimeSinceLastSprint >= StaminaRegenDelay)
		{
			if (StaminaStatus.Stamina < StaminaStatus.MaxStamina)
			{
				RegenerateStamina(DeltaTime);
			}
		}
	}
}

void UStaminaComponent::RequestStartSprint()
{
	if (GetOwnerRole() != ROLE_Authority)
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
	if (GetOwnerRole() != ROLE_Authority)
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
}

bool UStaminaComponent::IsCharacterSprinting() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	// Fall 상태에서는 Stamina를 소모해서는 안 된다.
	const bool bIsMoving = MovementComponent->IsSwimming() || MovementComponent->IsMovingOnGround();
	
	// LastInputVector는 Server에 전파가 안 된다.
	// 대신 Acceleration을 이용해서 판단한다.
	// Acceleration은 저항 등의 감속값은 계산하지 않기 때문에 입력 대용으로 사용할 수 있다.
	const bool bInputMove = MovementComponent->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;
	
	// 현재 캐릭터 애니메이션이 실제 속도에 비례해서 작동하기 때문에 실제 속도의 값도 계산해야 한다.
	// 가령, 캐릭터가 벽을 향해 달릴 경우 달리는 모션이 나오지 않기 때문에 스태미너를 소모해서는 안 된다.
	const bool bHasVelocity = MovementComponent->Velocity.SizeSquared() > KINDA_SMALL_NUMBER;

	return bIsMoving && bInputMove && bHasVelocity && OwnerCharacter->CanSprint();
}

void UStaminaComponent::OnRep_StaminaStatusChanged()
{
	OnStaminaChanged.Broadcast(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
	K2_OnStaminaChanged(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
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

void UStaminaComponent::ConsumeStamina(const float DeltaTime)
{
	const float NewStamina = StaminaStatus.Stamina - SprintConsumeRate * DeltaTime;
	SetStamina(NewStamina);

	if (bIsSprinting && StaminaStatus.Stamina <= 0.0f)
	{
		StopSprint();
	}
}

void UStaminaComponent::RegenerateStamina(const float DeltaTime)
{
	const float NewStamina = StaminaStatus.Stamina + SprintRegenRate * DeltaTime;
	SetStamina(NewStamina);
}

void UStaminaComponent::InitStamina(float MaxStamina, float Stamina)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	StaminaStatus.MaxStamina = MaxStamina;
	SetStamina(Stamina);
}

void UStaminaComponent::SetMaxStamina(const float NewMaxStamina)
{
	if (GetOwnerRole() != ROLE_Authority || NewMaxStamina <= 0 || NewMaxStamina == StaminaStatus.MaxStamina)
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
		OnStaminaChanged.Broadcast(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
		K2_OnStaminaChanged(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
	}
}

void UStaminaComponent::SetStamina(const float NewStamina)
{
	StaminaStatus.Stamina = FMath::Clamp(NewStamina, 0.0f, StaminaStatus.MaxStamina);
	OnStaminaChanged.Broadcast(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
	K2_OnStaminaChanged(StaminaStatus.Stamina, StaminaStatus.MaxStamina);
}

