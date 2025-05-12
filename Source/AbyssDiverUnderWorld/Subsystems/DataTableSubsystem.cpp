#include "Subsystems/DataTableSubsystem.h"

#include "Subsystems/ADTestGameInstance.h"
#include "DataRow/ADUpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"


void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADTestGameInstance* GI = CastChecked<UADTestGameInstance>(GetGameInstance());
	//ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("TestShopItemData"), DataTableArray);
	GI->ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("ItemDataTable"), ItemDataTableArray);
	GI->UpgradeDataTable->GetAllRows<FADUpgradeDataRow>(TEXT("UpgradeDataTable"), UpgradeDataTableArray);
	GI->OreDropTable->GetAllRows<FDropEntry>(TEXT("OreDropDataTable"), OreDropEntryTableArray);

	Algo::Sort(ItemDataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

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
