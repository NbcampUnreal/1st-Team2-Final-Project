// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Shops/ShopInteractionComponent.h"
#include "Shops/Shop.h"

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
		UpgradeGradeMap.Init(DefaultGrade, static_cast<int>(EUpgradeType::Max));
	}
}

void UUpgradeComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void UUpgradeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UUpgradeComponent, UpgradeGradeMap);
}

void UUpgradeComponent::S_RequestUpgrade_Implementation(EUpgradeType UpgradeType)
{
	bool bIsSucceeded = Upgrade(UpgradeType);
	if (bIsSucceeded)
	{
		OnRep_UpgradeGradeMap();
	}
}

void UUpgradeComponent::OnRep_UpgradeGradeMap()
{
	AADPlayerState* PS = Cast<AADPlayerState>(GetOwner());
	if (PS == nullptr)
	{
		LOGV(Error, TEXT("PS == nullptr"));
		return;
	}

	if (PS->GetPlayerController()->IsLocalController() == false)
	{
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PS->GetPlayerController()->GetPawn());
	if (PlayerCharacter == nullptr)
	{
		LOGV(Error, TEXT("PlayerCharacter == nullptr"));
		return;
	}

	UShopInteractionComponent* ShopInteractionComp = PlayerCharacter->GetShopInteractionComponent();
	if (ShopInteractionComp == nullptr)
	{
		LOGV(Error, TEXT("ShopInteractionComp == nullptr"));
		return;
	}

	AShop* Shop = ShopInteractionComp->GetCurrentInteractingShop();
	if (Shop == nullptr)
	{
		LOGV(Error, TEXT("Shop == nullptr"));
		return;
	}

	Shop->InitUpgradeView();
}

uint8 UUpgradeComponent::GetCurrentGrade(EUpgradeType UpgradeType) const
{
	const int32 Index = static_cast<int32>(UpgradeType);
	return UpgradeGradeMap.IsValidIndex(Index) ? UpgradeGradeMap[Index] : 0;
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

	int32 Index = static_cast<int32>(UpgradeType);
	if (UpgradeGradeMap.IsValidIndex(Index))
	{
		UpgradeGradeMap[Index]++;
		OnUpgradePerformed.Broadcast(UpgradeType, UpgradeGradeMap[Index]);
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

	int32 Index = static_cast<int32>(UpgradeType);
	UpgradeGradeMap[Index] = Grade;
	
	return true;
}

int32 UUpgradeComponent::GetUpgradeCost(EUpgradeType UpgradeType) const
{
	if (!DataTableSubsystem.IsValid())
	{
		LOGV(Error, TEXT("DataTableSubsystem is not valid"));
		return -1;
	}

	int32 Index = static_cast<int32>(UpgradeType);
	if (!UpgradeGradeMap.IsValidIndex(Index))
	{
		LOGV(Error, TEXT("UpgradeType is not valid"));
		return -1;
	}
	const int CurrentGrade = UpgradeGradeMap[Index];
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

	int32 Index = static_cast<int32>(UpgradeType);
	if (!UpgradeGradeMap.IsValidIndex(Index))
	{
		LOGV(Error, TEXT("UpgradeType is not valid"));
		return false;
	}
	const uint8 NextGrade = UpgradeGradeMap[Index] + 1;
	return DataTableSubsystem->GetUpgradeData(UpgradeType, NextGrade) == nullptr;
}

