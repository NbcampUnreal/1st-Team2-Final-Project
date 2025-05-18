#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataTableSubsystem.generated.h"

enum class EUpgradeType : uint8;
struct FFADItemDataRow;
struct FUpgradeDataRow;
struct FDropEntry;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UDataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Method
	
public:

	FFADItemDataRow* GetItemData(int32 ItemId) const;
	FUpgradeDataRow* GetUpgradeDataTableArray(int32 Index) const;
	FDropEntry* GetOreDropEntryTableArray(int32 Id) const;

	FUpgradeDataRow* GetUpgradeData(EUpgradeType UpgradeType, uint8 Grade) const;
	
private:
	void ParseUpgradeDataTable(class UADGameInstance* GameInstance);
	
#pragma endregion

#pragma region Variable

private:

	TArray<struct FFADItemDataRow*> ItemDataTableArray;
	TArray<struct FUpgradeDataRow*> UpgradeTableArray;
	TMap<TPair<EUpgradeType, uint8>, FUpgradeDataRow*> UpgradeTableMap;
	TArray<struct FDropEntry*> OreDropEntryTableArray;

#pragma endregion

#pragma region Getter/Setter

public:
	const TArray<struct FFADItemDataRow*>& GetItemDataTableArray() { return ItemDataTableArray; };

#pragma endregion
};
