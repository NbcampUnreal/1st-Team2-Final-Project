#include "UpgradeComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "Character/UnderwaterCharacter.h"
#include "Shops/ShopInteractionComponent.h"
#include "Shops/Shop.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UUpgradeComponent::UUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 업그레이드 정보가 Client에서도 필요하므로 해당 정보를 Replicate 해야 한다.
	SetIsReplicatedByDefault(true);

	UpgradeGradeMap.Init(DefaultGrade, static_cast<int>(EUpgradeType::Max));
}

void UUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		DataTableSubsystem = GameInstance->GetSubsystem<UDataTableSubsystem>();
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
	if (DataTableSubsystem.IsValid() == false)
	{
		LOGV(Warning, TEXT("DataTableSubsystem.IsValid() == false"));
		return;
	}
	bool bIsMaxGrade = IsMaxGrade(UpgradeType);
	if (bIsMaxGrade)
	{
		LOGV(Log, TEXT("This Upgrade Type Has Reached to Max Level"));
		return;
	}

	AADInGameState* GS = CastChecked<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));

	int32 TotalTeamCredit = GS->GetTotalTeamCredit();

	int32 Grade = GetGradeByType(UpgradeType);

	FUpgradeDataRow* UpgradeData = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade + 1);
	if (TotalTeamCredit < UpgradeData->Price)
	{
		LOGV(Warning, TEXT("업그레이드 돈부족!! 남은 돈 : %d, 필요한 돈 : %d"), TotalTeamCredit, UpgradeData->Price);
		return;
	}

	bool bIsSucceeded = Upgrade(UpgradeType);
	if (bIsSucceeded)
	{
		OnRep_UpgradeGradeMap();
		GS->SetTotalTeamCredit(TotalTeamCredit - UpgradeData->Price);
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

	APlayerController* PC = PS->GetPlayerController();
	if (PC == nullptr)
	{
		return;
	}

	if (PC->IsLocalController() == false)
	{
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PC->GetPawn());
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

void UUpgradeComponent::CopyProperties(UUpgradeComponent* Other)
{
	if (Other == nullptr)
	{
		LOGV(Error, TEXT("Other == nullptr"));
		return;
	}

	// UpgradeGradeMap을 복사한다.
	UpgradeGradeMap = Other->UpgradeGradeMap;
}

uint8 UUpgradeComponent::GetGradeByType(EUpgradeType UpgradeType) const
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
	if (UpgradeGradeMap.IsValidIndex(Index) && TrySetCurrentGrade(UpgradeType, UpgradeGradeMap[Index] + 1))
	{
		OnUpgradePerformed.Broadcast(UpgradeType, UpgradeGradeMap[Index]);
	}
	else
	{
		LOGV(Error, TEXT("UpgradeType is not valid"));
	}
	return true;
}

bool UUpgradeComponent::TrySetCurrentGrade(EUpgradeType UpgradeType, uint8 Grade)
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

const TArray<uint8>& UUpgradeComponent::GetUpgradeGradeMap() const
{
	return UpgradeGradeMap;
}

