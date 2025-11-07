#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataTableSubsystem.generated.h"

struct FMapDepthRow;
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
struct FGameGuideInfoRow;

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

	FGameGuideInfoRow* GetGameGuideInfo(int32 GuideId) const;

	// 유효하지 않으면 "invalid" 반환
	FString GetMapPath(EMapName MapName) const;

	FShopItemMeshTransformRow* GetShopItemMeshTransformData(int32 ItemId) const;

private:

	void ParseUpgradeDataTable(class UADGameInstance* GameInstance);
	void ParsePhaseGoalDataTable(class UADGameInstance* GameInstance);
	void ParseMapPathDataTable(class UADGameInstance* GameInstance);
	void ParseShopItemMeshTransformDataTable(class UADGameInstance* GameInstance);

#pragma endregion

#pragma region Variable

private:

	TArray<FFADItemDataRow*> ItemDataTableArray;
	TMap<uint8, FFADItemDataRow*> ItemDataTableMap;

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
	TMap<uint8, FShopItemMeshTransformRow*> ShopItemMeshTransformTableMap;

	TArray<FGameGuideInfoRow*> GameGuideInfoTableArray;

#pragma endregion

#pragma region Getter/Setter

public:
	const TArray<FFADItemDataRow*>& GetItemDataTableArray() { return ItemDataTableArray; };
	const TArray<FGameGuideInfoRow*>& GetGameGuideInfoTableArray() { return GameGuideInfoTableArray; };

	/** Map 이름에 따라 Depth Zone 정보를 반환 */
	FMapDepthRow* GetDepthZoneDataRow(FName MapName) const;
	
#pragma endregion
};
