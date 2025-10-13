#include "Subsystems/DataTableSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"
#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"
#include "DataRow/FADProjectileDataRow.h"
#include "DataRow/ButtonDataRow.h"
#include "DataRow/MapDepthRow.h"
#include "DataRow/PhaseGoalRow.h"
#include "DataRow/ShopItemMeshTransformRow.h"
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

	if (UDataTable* ProjectileDataTable = GI->ProjectileDataTable)
	{
		ProjectileDataTable->GetAllRows<FFADProjectileDataRow>(TEXT("ItemDataTable"), ProjectileDataTableArray);
	}

	if (UDataTable* ButtonDataTable = GI->ButtonDataTable)
	{
		ButtonDataTable->GetAllRows<FButtonDataRow>(TEXT("ButtonDataTable"), ButtonDataTableArray);
	}
	else
	{
		LOGV(Error, TEXT("ButtonDataTable is null"));
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

	Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

	Algo::Sort(ProjectileDataTableArray, [](const FFADProjectileDataRow* A, const FFADProjectileDataRow* B)
		{
			return A->Id < B->Id;
		});

	Algo::Sort(ButtonDataTableArray, [](const FButtonDataRow* A, const FButtonDataRow* B)
		{
			return A->Id < B->Id;
		});
	
	Algo::Sort(ShopItemMeshTransformTableArray, [](const FShopItemMeshTransformRow* A, const FShopItemMeshTransformRow* B)
		{
			return A->ItemId < B->ItemId;
		});

	ParsePhaseGoalDataTable(GI);
	ParseMapPathDataTable(GI);
	ParseShopItemMeshTransformDataTable(GI);
}


FFADItemDataRow* UDataTableSubsystem::GetItemData(int32 ItemId) const
{
	return ItemDataTableArray.IsValidIndex(ItemId) ? ItemDataTableArray[ItemId] : nullptr;
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

FFADProjectileDataRow* UDataTableSubsystem::GetProjectileData(int32 ProjectileId) const
{
	return ProjectileDataTableArray[ProjectileId];
}

FButtonDataRow* UDataTableSubsystem::GetButtonData(int32 ProjectileId) const
{
	return ButtonDataTableArray[ProjectileId];
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

FString UDataTableSubsystem::GetMapPath(EMapName MapName) const
{
	if (MapPathDataTableMap.Contains(MapName) == false)
	{
		LOGV(Error, TEXT("%d is not valid map name"), MapName);
		return "invalid";
	}

	return MapPathDataTableMap[MapName];
}

FShopItemMeshTransformRow* UDataTableSubsystem::GetShopItemMeshTransformData(int32 ItemId) const
{
	return ShopItemMeshTransformTableMap[ItemId];
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

	LOGV(Log, TEXT("PhaseGoalTableMap size: %d"), PhaseGoalTableMap.Num());
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

	LOGV(Log, TEXT("MapPathDataTableMap size: %d"), MapPathDataTableMap.Num());
}

void UDataTableSubsystem::ParseShopItemMeshTransformDataTable(UADGameInstance* GameInstance)
{
	if (GameInstance == nullptr || GameInstance->ShopMeshTransformTable == nullptr)
	{
		LOGV(Error, TEXT("GameInstance or MapPathDataTable is null"));
		return;
	}

	UDataTable* ShopMeshTransformTable = GameInstance->ShopMeshTransformTable;
	ShopMeshTransformTable->GetAllRows<FShopItemMeshTransformRow>(TEXT("ShopMeshTransformTable"), ShopItemMeshTransformTableArray);

	ShopItemMeshTransformTableMap.Empty(ShopItemMeshTransformTableArray.Num());
	for (FShopItemMeshTransformRow* Row : ShopItemMeshTransformTableArray)
	{
		if (Row == nullptr)
		{
			continue;
		}

		ShopItemMeshTransformTableMap.Add(Row->ItemId, Row);
	}

	LOGV(Log, TEXT("ShopItemMeshTransformTableMap size: %d"), ShopItemMeshTransformTableMap.Num());
}

FMapDepthRow* UDataTableSubsystem::GetDepthZoneDataRow(FName MapName) const
{
	UADGameInstance* GameInstance = Cast<UADGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		LOGV(Error, TEXT("GameInstance is null"));
		return nullptr;
	}
	
	const UDataTable* MapDepthTable = GameInstance->MapDepthTable;
	if (MapDepthTable == nullptr)
	{
		LOGV(Error, TEXT("MapDepthTable is null"));
		return nullptr;
	}
		 
	static const FString Context(TEXT("DepthZoneDataTable"));
	return MapDepthTable->FindRow<FMapDepthRow>(MapName, Context);
}
