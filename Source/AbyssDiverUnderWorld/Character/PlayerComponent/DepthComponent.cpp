// Fill out your copyright notice in the Description page of Project Settings.


#include "DepthComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "DataRow/MapDepthRow.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/ADWorldSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"


// Sets default values for this component's properties
UDepthComponent::UDepthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bIsActive = true;
	ReferenceZ = 0.0f;
	ReferenceDepth = 0.0f;
	WarningZoneZ = 0.0f;
	DangerZoneZ = 0.0f;
	CurrentDepthZone = EDepthZone::SafeZone;
}

// Called every frame
void UDepthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDepthComponent::GetMapDepthData()
{
	UADGameInstance* GameInstance = Cast<UADGameInstance>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("Depth: Failed to get GameInstance"));
		return;
	}

	UDataTableSubsystem* DataTableSubsystem = GameInstance->GetSubsystem<UDataTableSubsystem>();
	if (!DataTableSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Depth: Failed to get DataTableSubsystem"));
		return;
	}

	UADWorldSubsystem* WorldSubsystem = GetWorld()->GetSubsystem<UADWorldSubsystem>();
	if (!WorldSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Depth: Failed to get WorldSubsystem"));
		return;
	}
	
	FName MapName = FName(WorldSubsystem->GetCurrentLevelName());
	const FMapDepthRow* MapDepthRow = DataTableSubsystem->GetDepthZoneDataRow(MapName);
	if (!MapDepthRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get MapDepthRow for %s"), *UEnum::GetValueAsString(GameInstance->SelectedLevelName));
		return;
	}

	ReferenceZ = MapDepthRow->ReferenceZ;
	ReferenceDepth = MapDepthRow->ReferenceDepth;
	WarningZoneZ = MapDepthRow->WarningZoneZ;
	DangerZoneZ = MapDepthRow->DangerZoneZ;
	UE_LOG(LogAbyssDiverCharacter, Display,
		TEXT("Depth Component: ReferenceZ = %f, ReferenceDepth = %f, WarningZoneZ = %f, DangerZoneZ = %f"),
		ReferenceZ, ReferenceDepth, WarningZoneZ, DangerZoneZ);
}

void UDepthComponent::BeginPlay()
{
	Super::BeginPlay();

	LOGVN(Display, TEXT("AADInGameState Related: Depth Component BeginPlay called for %s"), *GetOwner()->GetName());
	
	if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(GetOwner()))
	{
		Character->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UDepthComponent::OnEnvironmentStateChanged);
	}
	
	GetMapDepthData();
	CurrentDepthZone = GetCurrentZone(GetOwner()->GetActorLocation().Z);
}

EDepthZone UDepthComponent::GetCurrentZone(float CurrentZ) const
{
	if (CurrentZ < DangerZoneZ)
	{
		return EDepthZone::DangerZone;
	}
	else if (CurrentZ < WarningZoneZ)
	{
		return EDepthZone::WarningZone;
	}

	return EDepthZone::SafeZone;
}

void UDepthComponent::UpdateZone()
{
	// 현재 Actor의 깊이를 계산한다.
	float CurrentZ = GetOwner()->GetActorLocation().Z;
	EDepthZone NewDepthZone = bIsActive ? GetCurrentZone(CurrentZ) : EDepthZone::SafeZone;

	if (NewDepthZone != CurrentDepthZone)
	{
		EDepthZone OldDepthZone = CurrentDepthZone;
		CurrentDepthZone = NewDepthZone;

		OnDepthZoneChangedDelegate.Broadcast(OldDepthZone, NewDepthZone);
	}
}

void UDepthComponent::OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState,
	EEnvironmentState NewEnvironmentState)
{
	const bool ShouldActiveDepth = NewEnvironmentState == EEnvironmentState::Underwater;
	SetDepthZoneActive(ShouldActiveDepth);

	UpdateZone();
}

void UDepthComponent::SetDepthZoneActive(bool bActive)
{
	bIsActive = bActive;
}

float UDepthComponent::GetCurrentDepth() const
{
	// 현재 Actor의 Z 위치를 기준으로 깊이를 계산한다.
	// ReferenceZ와 비교해서 깊이를 반환한다.
	return GetOwner()->GetActorLocation().Z - ReferenceZ + ReferenceDepth;
}


