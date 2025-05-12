// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DataTableSubsystem.h"

// Sets default values for this component's properties
UUpgradeComponent::UUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 업그레이드 정보가 Client에서도 필요하므로 해당 정보를 Replicate 해야 한다.
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		DataTableSubsystem = GameInstance->GetSubsystem<UDataTableSubsystem>();
	}

	// To-DO
	// 1. Server Travel 시에 데이터가 제대로 전송되는지 확인할 것
	// 2. Copy Properties 정보가 들어오는 시점을 확인해서 처리할 것
	if (UpgradeGradeMap.IsEmpty())
	{
		for (int32 i = 0; i < static_cast<int32>(EUpgradeType::Max); ++i)
		{
			UpgradeGradeMap.Add(static_cast<EUpgradeType>(i), DefaultGrade);
		}
	}
}

void UUpgradeComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

uint8 UUpgradeComponent::GetCurrentGrade(EUpgradeType UpgradeType) const
{
	return UpgradeGradeMap.FindRef(UpgradeType);
}

bool UUpgradeComponent::Upgrade(EUpgradeType UpgradeType)
{
	if (!DataTableSubsystem.IsValid())
	{
		LOGV(Error, TEXT("DataTableSubsystem is not valid"));
		return false;
	}
	if (IsMaxGrade(UpgradeType))
	{
		return false;
	}

	if (uint8* CurrentGrade = UpgradeGradeMap.Find(UpgradeType))
	{
		(*CurrentGrade)++;
		OnUpgradePerformed.Broadcast(UpgradeType, *CurrentGrade);
	}
	else
	{
		LOGV(Error, TEXT("UpgradeType is not valid"));
	}
	return true;
}

bool UUpgradeComponent::SetCurrentGrade(EUpgradeType UpgradeType, uint8 Grade)
{
	if (!DataTableSubsystem.IsValid())
	{
		LOGV(Error, TEXT("DataTableSubsystem is not valid"));
		return false;
	}
	if (DataTableSubsystem->GetUpgradeData(UpgradeType, Grade) == nullptr)
	{
		return false;
	}

	UpgradeGradeMap.Add(UpgradeType, Grade);
	return true;
}

int32 UUpgradeComponent::GetUpgradeCost(EUpgradeType UpgradeType) const
{
	if (!DataTableSubsystem.IsValid())
	{
		LOGV(Error, TEXT("DataTableSubsystem is not valid"));
		return -1;
	}

	const int CurrentGrade = UpgradeGradeMap.FindRef(UpgradeType);
	const FUpgradeDataRow* UpgradeData = DataTableSubsystem->GetUpgradeData(UpgradeType, CurrentGrade);
	return UpgradeData ? UpgradeData->Price : -1;
}

bool UUpgradeComponent::IsMaxGrade(EUpgradeType UpgradeType) const
{
	if (!DataTableSubsystem.IsValid())
	{
		LOGV(Error, TEXT("DataTableSubsystem is not valid"));
		return false;
	}

	const uint8 NextGrade = UpgradeGradeMap.FindRef(UpgradeType) + 1;
	return DataTableSubsystem->GetUpgradeData(UpgradeType, NextGrade) == nullptr;
}

