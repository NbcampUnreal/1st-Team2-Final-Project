#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataTableSubsystem.generated.h"

enum class EUpgradeType : uint8;
enum class EMapName : uint8;
struct FFADItemDataRow;
struct FFADProjectileDataRow;
struct FButtonDataRow;
struct FUpgradeDataRow;
struct FDropEntry;
struct FPhaseGoalRow;
struct FMapPathDataRow;
struct FShopItemMeshTransformRow;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UDataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Method
	
public:

	FFADItemDataRow* GetItemData(int32 ItemId) const;
	FFADItemDataRow* GetItemDataByName(FName ItemName) const;
	FFADProjectileDataRow* GetProjectileData(int32 ProjectileId) const;
	FButtonDataRow* GetButtonData(int32 ProjectileId) const;
	FUpgradeDataRow* GetUpgradeDataTableArray(int32 Index) const;
	FDropEntry* GetOreDropEntryTableArray(int32 Id) const;

	FUpgradeDataRow* GetUpgradeData(EUpgradeType UpgradeType, uint8 Grade) const;

	FPhaseGoalRow* GetPhaseGoalData(EMapName MapName, int32 Phase) const;

	const FString& GetMapPath(EMapName MapName) const;

	FShopItemMeshTransformRow* GetShopItemMeshTransformData(int32 ItemId) const;

private:
	void ParseUpgradeDataTable(class UADGameInstance* GameInstance);
	void ParsePhaseGoalDataTable(class UADGameInstance* GameInstance);
	void ParseMapPathDataTable(class UADGameInstance* GameInstance);
	
#pragma endregion

#pragma region Variable

private:

	TArray<FFADItemDataRow*> ItemDataTableArray;
	TArray<FFADProjectileDataRow*> ProjectileDataTableArray;
	TArray<FButtonDataRow*> ButtonDataTableArray;
	TArray<FUpgradeDataRow*> UpgradeTableArray;
	TMap<TPair<EUpgradeType, uint8>, FUpgradeDataRow*> UpgradeTableMap;
	TArray<FDropEntry*> OreDropEntryTableArray;

	TArray<FPhaseGoalRow*> PhaseGoalTableArray;
	TMap<TPair<EMapName, int32>, FPhaseGoalRow*> PhaseGoalTableMap;

	TArray<FMapPathDataRow*> MapPathDataTableArray;
	TMap<EMapName, FString> MapPathDataTableMap;

	TArray<FShopItemMeshTransformRow*> ShopItemMeshTransformTableArray;

#pragma endregion

#pragma region Getter/Setter

public:
	const TArray<FFADItemDataRow*>& GetItemDataTableArray() { return ItemDataTableArray; };

#pragma endregion
};
