#include "Subsystems/DataTableSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"
#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"
#include "DataRow/PhaseGoalRow.h"
#include "Interactable/Item/ADOreRock.h"
#include "Logging/LogMacros.h"


void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADGameInstance* GI = CastChecked<UADGameInstance>(GetGameInstance());
	//ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("TestShopItemData"), DataTableArray);
	if (UDataTable* ItemDataTable = GI->ItemDataTable)
	{
		ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("ItemDataTable"), ItemDataTableArray);
	}
	else
	{
		LOGV(Error, TEXT("ItemDataTable is null"));
	}
	
	ParseUpgradeDataTable(GI);
	
	if (UDataTable* OreDropTable = GI->OreDropTable)
	{
		OreDropTable->GetAllRows<FDropEntry>(TEXT("OreDropDataTable"), OreDropEntryTableArray);
	}
	else
	{
		LOGV(Error, TEXT("OreDropTable is null"));
	}

	//Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
	//	{
	//		return A->Id < B->Id;
	//	});

	ParsePhseGoalDataTable(GI);

}

int8 UDataTableSubsystem::GetItemDataTableArrayNum() const
{
	return ItemDataTableArray.Num();
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
		return ItemDataTable->FindRow<FFADItemDataRow>(ItemName, TEXT("Lookup Item"));
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

void UDataTableSubsystem::ParsePhseGoalDataTable(UADGameInstance* GameInstance)
{
	if (GameInstance == nullptr || GameInstance->PhaseGoalTable == nullptr)
	{
		LOGV(Error, TEXT("GameInstance or PhaseGoalDataTable is null"));
		return;
	}

	UDataTable* PhaseGoalTable = GameInstance->UpgradeDataTable;
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
