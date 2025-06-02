#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataRow/MissionDataRow/AggroTriggerMissionRow.h"
#include "DataRow/MissionDataRow/InteractionMissionRow.h"
#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"
#include "DataRow/MissionDataRow/ItemUseMissionRow.h"
#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"
#include "UI/MissionData.h"

#include "MissionSubsystem.generated.h"

class UAggroTriggerMission;
class UInteractionMission;
class UItemCollectionMission;
class UItemUseMission;
class UKillMonsterMission;
class UMissionBase;

class AADInGameState;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Methods

public:

	void AddToSelectedMissions(const EAggroTriggerMission& Mission);
	void AddToSelectedMissions(const EInteractionMission& Mission);
	void AddToSelectedMissions(const EItemCollectMission& Mission);
	void AddToSelectedMissions(const EItemUseMission& Mission);
	void AddToSelectedMissions(const EKillMonsterMission& Mission);

	// return : Succeeded?
	bool AddToSelectedMissionFromIndex(const EMissionType& MissionType, const uint8& MissionIndex);

	void RemoveFromSelectedMissions(const EAggroTriggerMission& Mission);
	void RemoveFromSelectedMissions(const EInteractionMission& Mission);
	void RemoveFromSelectedMissions(const EItemCollectMission& Mission);
	void RemoveFromSelectedMissions(const EItemUseMission& Mission);
	void RemoveFromSelectedMissions(const EKillMonsterMission& Mission);

	void UnlockMission(const EAggroTriggerMission& Mission);
	void UnlockMission(const EInteractionMission& Mission);
	void UnlockMission(const EItemCollectMission& Mission);
	void UnlockMission(const EItemUseMission& Mission);
	void UnlockMission(const EKillMonsterMission& Mission);

	void ClearSelectedMissions(const int32& SlackCount = 0);

	void ReceiveMissionDataFromUIData(const TArray<FMissionData>& Missions);

private:

	void MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData, const uint8& MissionIndex);

	// return : Succeeded?
	bool RemoveFromSelectedMissionsInternal(const EMissionType& MissionType, const uint8& MissionIndex);

	bool IsAlreadySelected(const EMissionType& MissionType, const uint8& MissionIndex) const;
	bool IsServer() const;

	void UnlockMissionInternal(FMissionBaseRow* MissionsFromUI);

	bool CheckIfGameStateIsValid();

#pragma endregion

#pragma region Variables

private:

	UPROPERTY()
	TArray<TObjectPtr<UMissionBase>> Missions;

	// 미션 타입에 맞게 uint8을 Enum으로 캐스팅하여 사용
	// 이것을 토대로 미션 생성할 예정
	TMap<EMissionType, TArray<uint8>> SelectedMissions;
	// Set을 쓰기 때문에 미션 이름이 같은게 있으면 안 된다.
	TSet<FString> SelecedMissionNames;

	TSet<FMissionData> MissionDataForUI;

	TArray<FAggroTriggerMissionRow*> AggroTriggerMissionData;
	TArray<FInteractionMissionRow*> InteractionMissionData;
	TArray<FItemCollectMissionRow*> ItemCollectMissionData;
	TArray<FItemUseMissionRow*> ItemUseMissionData;
	TArray<FKillMonsterMissionRow*> KillMonsterMissionData;

	int32 AggroTriggerMissionCount = 0;
	int32 InteractionMissionCount = 0;
	int32 ItemCollectMissionCount = 0;
	int32 ItemUseMissionCount = 0;
	int32 KillMonsterMissionCount = 0;

	TObjectPtr<AADInGameState> InGameState;

#pragma endregion

#pragma region Getters / Setters
public:

	const TSet<FString>& GetAllSelectedMissionNames() const;

	const TSet<FMissionData>& GetMissionDataForUI() const;
	const TArray<TObjectPtr<UMissionBase>>& GetActivatedMissions() const;

	const FAggroTriggerMissionRow* GetAggroTriggerMissionData(const EAggroTriggerMission& Mission) const;
	const FInteractionMissionRow* GetInteractionMissionData(const EInteractionMission& Mission) const;
	const FItemCollectMissionRow* GetItemCollectMissionData(const EItemCollectMission& Mission) const;
	const FItemUseMissionRow* GetItemUseMissionData(const EItemUseMission& Mission) const;
	const FKillMonsterMissionRow* GetKillMonsterMissionData(const EKillMonsterMission& Mission) const;
#pragma endregion

};
