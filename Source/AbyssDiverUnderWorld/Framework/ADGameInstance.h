#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AdvancedFriendsGameInstance.h"
#include "UI/MissionData.h"
#include "Framework/SettingsManager.h"
#include "ADGameInstance.generated.h"

enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADGameInstance : public UAdvancedFriendsGameInstance
{
    GENERATED_BODY()
public:
    void SetSelectedMissions(const TArray<FMissionData>& Missions) { SelectedMissions = Missions; }
    const TArray<FMissionData>& GetSelectedMissions() const { return SelectedMissions; }

public:
	UADGameInstance(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void Init() override;

public:

	bool TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex);
	void AddPlayerNetId(const FString& NetId);
	void RemovePlayerNetId(const FString& NetId);
	
	// 0~1의 값
	UFUNCTION(BlueprintCallable, Category = "ADGameInstance")
	void ChangeMasterVolume(const float& NewVolume);

	// 0~1의 값
	UFUNCTION(BlueprintCallable, Category = "ADGameInstance")
	void ChangeBGMVolume(const float& NewVolume);

	// 0~1의 값
	UFUNCTION(BlueprintCallable, Category = "ADGameInstance")
	void ChangeSFXVolume(const float& NewVolume);

	// 0~1의 값
	UFUNCTION(BlueprintCallable, Category = "ADGameInstance")
	void ChangeAmbientVolume(const float& NewVolume);

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

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.AmbientDataRow"))
	TObjectPtr<UDataTable> AmbientDataTable;

#pragma endregion

#pragma region Mission Tables

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance|MissionData", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.KillMonsterMissionRow"))
	TObjectPtr<UDataTable> KillMonsterMissionTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance|MissionData", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.ItemUseMissionRow"))
	TObjectPtr<UDataTable> ItemUseMissionTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance|MissionData", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.ItemCollectMissionRow"))
	TObjectPtr<UDataTable> ItemCollectMissionTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance|MissionData", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.InteractionMissionRow"))
	TObjectPtr<UDataTable> InteractionMissionTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance|MissionData", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.AggroTriggerMissionRow"))
	TObjectPtr<UDataTable> AggroTriggerMissionTable;

#pragma endregion

	//Settings
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USettingsManager> SettingsManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TMap<FName, UInputAction*> InputActionMap;

private:

	TMap<FString, int32> PlayerIdMap;
	TArray<bool> ValidPlayerIndexArray;

	const int32 MAX_PLAYER_NUMBER = 4;
	

#pragma region Getters / Setters
public:

	UFUNCTION(BlueprintPure, Category = "ADGameInstance")
	const float GetCurrentMasterVolume() const;

	UFUNCTION(BlueprintPure, Category = "ADGameInstance")
	const float GetCurrentBGMVolume() const;

	UFUNCTION(BlueprintPure, Category = "ADGameInstance")
	const float GetCurrentSFXVolume() const;

	UFUNCTION(BlueprintPure, Category = "ADGameInstance")
	const float GetCurrentAmbientVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	USettingsManager* GetSettingsManager() const { return SettingsManager; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	const TMap<FName, UInputAction*>& GetInputActionMap() const { return InputActionMap; }
#pragma endregion


};
