#include "Subsystems/DataTableSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/ADTestGameInstance.h"
#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"
#include "Interactable/Item/ADOreRock.h"
#include "Logging/LogMacros.h"


void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADTestGameInstance* GI = CastChecked<UADTestGameInstance>(GetGameInstance());
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

	Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

}

FFADItemDataRow* UDataTableSubsystem::GetItemData(int32 ItemId) const
{
	return ItemDataTableArray[ItemId];
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

void UDataTableSubsystem::ParseUpgradeDataTable(UADTestGameInstance* GameInstance)
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
