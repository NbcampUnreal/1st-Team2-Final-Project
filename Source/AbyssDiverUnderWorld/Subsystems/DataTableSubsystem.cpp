#include "Subsystems/DataTableSubsystem.h"

#include "Subsystems/ADTestGameInstance.h"
#include "DataRow/ADUpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"
#include "Interactable/Item/ADOreRock.h"


void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADTestGameInstance* GI = CastChecked<UADTestGameInstance>(GetGameInstance());
	//ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("TestShopItemData"), DataTableArray);
	if (UDataTable* ItemDataTable = GI->ItemDataTable)
	{
		ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("ItemDataTable"), ItemDataTableArray);
	}
	if (UDataTable* UpgradeDataTable = GI->UpgradeDataTable)
	{
		UpgradeDataTable->GetAllRows<FADUpgradeDataRow>(TEXT("UpgradeDataTable"), UpgradeDataTableArray);
	}
	if (UDataTable* OreDropTable = GI->OreDropTable)
	{
		OreDropTable->GetAllRows<FDropEntry>(TEXT("OreDropDataTable"), OreDropEntryTableArray);
	}

	//Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
	//	{
	//		return A->Id < B->Id;
	//	});

}

int8 UDataTableSubsystem::GetItemDataTableArrayNum() const
{
	return ItemDataTableArray.Num();
}

FFADItemDataRow* UDataTableSubsystem::GetItemData(int32 ItemId) const
{
	return ItemDataTableArray[ItemId];
}

FADUpgradeDataRow* UDataTableSubsystem::GetUpgradeDataTableArray(int32 Index) const
{
	return UpgradeDataTableArray[Index];
}

FDropEntry* UDataTableSubsystem::GetOreDropEntryTableArray(int32 Id) const
{
	return OreDropEntryTableArray[Id];
}
