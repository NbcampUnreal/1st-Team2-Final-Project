#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataTableSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UDataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:

	TArray<struct FFADItemDataRow*> ItemDataTableArray;
	TArray<struct FADUpgradeDataRow*> UpgradeDataTableArray;
	TArray<struct FDropEntry*> OreDropEntryTableArray;

public:

	int8 GetItemDataTableArrayNum() const ;
	FFADItemDataRow* GetItemData(int32 ItemId) const;
	FADUpgradeDataRow* GetUpgradeDataTableArray(int32 Index) const;
	FDropEntry* GetOreDropEntryTableArray(int32 Id) const;
};
