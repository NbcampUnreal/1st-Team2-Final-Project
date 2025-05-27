#include "Subsystems/DataTableSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"
#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"
#include "DataRow/FADProjectileDataRow.h"
#include "DataRow/PhaseGoalRow.h"
#include "DataRow/ShopItemMeshTransformRow.h"
#include "Interactable/Item/ADOreRock.h"
#include "Logging/LogMacros.h"

void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADGameInstance* GI = CastChecked<UADGameInstance>(GetGameInstance());
	if (UDataTable* ItemDataTable = GI->ItemDataTable)
	{
		ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("ItemDataTable"), ItemDataTableArray);
	}
	else
	{
		LOGV(Error, TEXT("ItemDataTable is null"));
	}

	if (UDataTable* ShopItemTransformTable = GI->ShopMeshTransformTable)
	{
		ShopItemTransformTable->GetAllRows<FShopItemMeshTransformRow>(TEXT("ShopItemMeshTransformRow"), ShopItemMeshTransformTableArray);
	}
	else
	{
		LOGV(Error, TEXT("ItemDataTable is null"));
	}
	
	ParseUpgradeDataTable(GI);
	
	//if (UDataTable* OreDropTable = GI->OreDropTable)
	//{
	//	OreDropTable->GetAllRows<FDropEntry>(TEXT("OreDropDataTable"), OreDropEntryTableArray);
	//}
	//else
	//{
	//	LOGV(Error, TEXT("OreDropTable is null"));
	//}

	Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

	Algo::Sort(ShopItemMeshTransformTableArray, [](const FShopItemMeshTransformRow* A, const FShopItemMeshTransformRow* B)
		{
			return A->ItemId < B->ItemId;
		});

	ParsePhaseGoalDataTable(GI);
	ParseMapPathDataTable(GI);



}


FFADItemDataRow* UDataTableSubsystem::GetItemData(int32 ItemId) const
{
	return ItemDataTableArray[ItemId];
}

FFADItemDataRow* UDataTableSubsystem::GetItemDataByName(FName ItemName) const
{
	UADGameInstance* GI = CastChecked<UADGameInstance>(GetGameInstance());
	if (UDataTable* ItemDataTable = GI->ItemDataTable)
	{
		return ItemDataTable->FindRow<FFADItemDataRow>(ItemName, TEXT("LookupItem"));
	}
	else
	{
		LOGV(Error, TEXT("ProjectileRow is null"));
	}
	return nullptr;
}

FFADProjectileDataRow* UDataTableSubsystem::GetProjectileDataArrayByName(FName ProjectileName) const
{
	UADGameInstance* GI = CastChecked<UADGameInstance>(GetGameInstance());
	if (UDataTable* ProjectileDataTable = GI->ProjectileDataTable)
	{
		return ProjectileDataTable->FindRow<FFADProjectileDataRow>(ProjectileName, TEXT("LookupItem"));
	}
	else
	{
		LOGV(Error, TEXT("ProjectileRow is null"));
	}
	return nullptr;
}

FUpgradeDataRow* UDataTableSubsystem::GetUpgradeDataTableArray(int32 Index) const
{
	return UpgradeTableArray[Index];
}

FDropEntry* UDataTableSubsystem::GetOreDropEntryTableArray(int32 Id) const
{
	return OreDropEntryTableArray[Id];
}

FUpgradeDataRow* UDataTableSubsystem::GetUpgradeData(EUpgradeType UpgradeType, uint8 Grade) const
{
	return UpgradeTableMap.FindRef(TPair<EUpgradeType, uint8>(UpgradeType, Grade));
}

FPhaseGoalRow* UDataTableSubsystem::GetPhaseGoalData(EMapName MapName, int32 Phase) const
{
	return PhaseGoalTableMap.FindRef(TPair<EMapName, int32>(MapName, Phase));
}

const FString& UDataTableSubsystem::GetMapPath(EMapName MapName) const
{
	return MapPathDataTableMap[MapName];
}

FShopItemMeshTransformRow* UDataTableSubsystem::GetShopItemMeshTransformData(int32 ItemId) const
{
	return ShopItemMeshTransformTableArray[ItemId];
}

void UDataTableSubsystem::ParseUpgradeDataTable(UADGameInstance* GameInstance)
{
	if (GameInstance == nullptr || GameInstance->UpgradeDataTable == nullptr)
	{
		LOGV(Error, TEXT("GameInstance or UpgradeDataTable is null"));
		return;
	}

	UDataTable* UpgradeDataTable = GameInstance->UpgradeDataTable;
	UpgradeDataTable->GetAllRows<FUpgradeDataRow>(TEXT("ItemDataTable"), UpgradeTableArray);

	UpgradeTableMap.Empty(UpgradeTableArray.Num());
	for (FUpgradeDataRow* Row : UpgradeTableArray)
	{
		if (Row == nullptr)
		{
			continue;
		}

		TPair<EUpgradeType, uint8> Key(Row->UpgradeType, Row->Grade);
		UpgradeTableMap.Add(Key, Row);
	}
}

void UDataTableSubsystem::ParsePhaseGoalDataTable(UADGameInstance* GameInstance)
{
	if (GameInstance == nullptr || GameInstance->PhaseGoalTable == nullptr)
	{
		LOGV(Error, TEXT("GameInstance or PhaseGoalDataTable is null"));
		return;
	}

	UDataTable* PhaseGoalTable = GameInstance->PhaseGoalTable;
	PhaseGoalTable->GetAllRows<FPhaseGoalRow>(TEXT("PhaseGoalTable"), PhaseGoalTableArray);

	PhaseGoalTableMap.Empty(PhaseGoalTableArray.Num());
	for (FPhaseGoalRow* Row : PhaseGoalTableArray)
	{
		if (Row == nullptr)
		{
			continue;
		}
		TPair<EMapName, int32> Key(Row->MapName, Row->Phase);
		PhaseGoalTableMap.Add(Key, Row);
	}

	LOGV(Error, TEXT("PhaseGoalTableMap size: %d"), PhaseGoalTableMap.Num());
}

void UDataTableSubsystem::ParseMapPathDataTable(UADGameInstance* GameInstance)
{
	if (GameInstance == nullptr || GameInstance->MapPathDataTable == nullptr)
	{
		LOGV(Error, TEXT("GameInstance or MapPathDataTable is null"));
		return;
	}

	UDataTable* MapPathTable = GameInstance->MapPathDataTable;
	MapPathTable->GetAllRows<FMapPathDataRow>(TEXT("MapPathTable"), MapPathDataTableArray);

	MapPathDataTableMap.Empty(MapPathDataTableArray.Num());
	for (FMapPathDataRow* Row : MapPathDataTableArray)
	{
		if (Row == nullptr)
		{
			continue;
		}

		MapPathDataTableMap.Add(Row->MapName, Row->MapPath);
	}

	LOGV(Error, TEXT("PhaseGoalTableMap size: %d"), MapPathDataTableMap.Num());
}
