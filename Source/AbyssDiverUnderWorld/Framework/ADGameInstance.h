#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MissionData.h"
#include "ADGameInstance.generated.h"

enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    void SetSelectedMissions(const TArray<FMissionData>& Missions) { SelectedMissions = Missions; }
    const TArray<FMissionData>& GetSelectedMissions() const { return SelectedMissions; }

public:
	UADGameInstance();

protected:

	virtual void Init() override;

public:
    // 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADItemDataRow"))
    TObjectPtr<UDataTable> ItemDataTable;

	bool TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex);
	void AddPlayerNetId(const FString& NetId);
	void RemovePlayerNetId(const FString& NetId);

public:
	UPROPERTY(BlueprintReadWrite)
	EMapName SelectedLevelName;

	UPROPERTY(BlueprintReadWrite)
	int32 TeamCredits;


	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADProjectileDataRow"))
	TObjectPtr<UDataTable> ProjectileDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.UpgradeDataRow"))
    TObjectPtr<UDataTable> UpgradeDataTable;

    // 선택된 미션을 저장할 배열
    UPROPERTY(BlueprintReadWrite)
    TArray<FMissionData> SelectedMissions;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.PhaseGoalRow"))
	TObjectPtr<UDataTable> PhaseGoalTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.MapPathDataRow"))
	TObjectPtr<UDataTable> MapPathDataTable;


private:

	TMap<FString, int32> PlayerIdMap;
	TArray<bool> ValidPlayerIndexArray;

	const int32 MAX_PLAYER_NUMBER = 4;

};
