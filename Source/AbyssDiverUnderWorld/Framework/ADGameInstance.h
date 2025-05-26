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
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ExitSession(APlayerController* Caller);

	bool TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex);
	void AddPlayerNetId(const FString& NetId);
	void RemovePlayerNetId(const FString& NetId);

	// 0~1의 값
	UFUNCTION(Exec, BlueprintCallable, Category = "ADGameInstance")
	void ChangeMasterVolume(const float& NewVolume) const;

	// 0~1의 값
	UFUNCTION(Exec, BlueprintCallable, Category = "ADGameInstance")
	void ChangeBGMVolume(const float& NewVolume) const;

	// 0~1의 값
	UFUNCTION(Exec, BlueprintCallable, Category = "ADGameInstance")
	void ChangeSFXVolume(const float& NewVolume) const;

public:
	UPROPERTY(BlueprintReadWrite)
	uint8 bIsHost : 1;

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

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.ShopItemMeshTransformRow"))
	TObjectPtr<UDataTable> ShopMeshTransformTable;

#pragma region Sound Tables

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.SFXDataRow"))
	TObjectPtr<UDataTable> SFXDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.UISFXDataRow"))
	TObjectPtr<UDataTable> UISFXDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.MonsterSFXDataRow"))
	TObjectPtr<UDataTable> MonsterSFXDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.BGMDataRow"))
	TObjectPtr<UDataTable> BGMDataTable;

#pragma endregion

private:

	TMap<FString, int32> PlayerIdMap;
	TArray<bool> ValidPlayerIndexArray;

	const int32 MAX_PLAYER_NUMBER = 4;

};
