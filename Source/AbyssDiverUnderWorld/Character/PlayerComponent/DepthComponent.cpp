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
	bUseWarningZone = false;
	bUseDangerZone = false;
	WarningZoneZ = 0.0f;
	DangerZoneZ = 0.0f;
	DepthZone = EDepthZone::SafeZone;
}

// Called every frame
void UDepthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateZone();
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
	bUseDangerZone = MapDepthRow->bUseDangerZone;
	bUseWarningZone = MapDepthRow->bUseWarningZone;
	WarningZoneZ = MapDepthRow->WarningZoneZ;
	DangerZoneZ = MapDepthRow->DangerZoneZ;
	UE_LOG(LogAbyssDiverCharacter, Display,
		TEXT("Depth Component: ReferenceZ = %f, ReferenceDepth = %f, WarningZoneZ = %f, DangerZoneZ = %f"),
		ReferenceZ, ReferenceDepth, WarningZoneZ, DangerZoneZ);
}

void UDepthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(GetOwner()))
	{
		Character->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UDepthComponent::OnEnvironmentStateChanged);
	}
	
	GetMapDepthData();
	DepthZone = DetermineZone(GetOwner()->GetActorLocation().Z);
}


EDepthZone UDepthComponent::DetermineZone(float CurrentZ) const
{
	if (bUseDangerZone && CurrentZ < DangerZoneZ)
	{
		return EDepthZone::DangerZone;
	}
	else if (bUseWarningZone && CurrentZ < WarningZoneZ)
	{
		return EDepthZone::WarningZone;
	}

	return EDepthZone::SafeZone;
}

void UDepthComponent::UpdateZone()
{
	// 현재 Actor의 깊이를 계산한다.
	float CurrentZ = GetOwner()->GetActorLocation().Z;
	EDepthZone NewDepthZone = bIsActive ? DetermineZone(CurrentZ) : EDepthZone::SafeZone;
	if (OnDepthUpdatedDelegate.IsBound())
	{
		OnDepthUpdatedDelegate.Broadcast(GetDepth());
	}
	else
		LOGN(TEXT("OnDepthUpdatedDelegate Is Not Bound"));

	if (NewDepthZone != DepthZone)
	{
		UE_LOG(LogAbyssDiverCharacter, Display,
			TEXT("Depth Component: DepthZone changed from %s to %s for %s"),
			*UEnum::GetValueAsString(DepthZone),
			*UEnum::GetValueAsString(NewDepthZone),
			*GetOwner()->GetName());
		EDepthZone OldDepthZone = DepthZone;
		DepthZone = NewDepthZone;
		if (OnDepthZoneChangedDelegate.IsBound())
		{
			OnDepthZoneChangedDelegate.Broadcast(OldDepthZone, NewDepthZone);
		}
		else
			LOGN(TEXT("OnDepthZoneChangedDelegate Is Not Bound"));
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

float UDepthComponent::GetDepth() const
{
	// 현재 Actor의 Z 위치를 기준으로 깊이를 계산한다.
	// ReferenceZ와 비교해서 깊이를 반환한다.
	return (ReferenceZ - GetOwner()->GetActorLocation().Z + ReferenceDepth) / 100.0f; // cm to m conversion
}


