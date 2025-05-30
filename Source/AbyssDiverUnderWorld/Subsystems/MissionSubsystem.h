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

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Methods

public:

	void AddToSelectedMissions(const EAggroTriggerMission& Mission);
	void AddToSelectedMissions(const EInteractionMission& Mission);
	void AddToSelectedMissions(const EItemCollectMission& Mission);
	void AddToSelectedMissions(const EItemUseMission& Mission);
	void AddToSelectedMissions(const EKillMonsterMission& Mission);

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

private:

	// return : Succeeded?
	bool AddToSelectedMissionInternal(const EMissionType& MissionType, const uint8& MissionIndex);
	void MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData);

	// return : Succeeded?
	bool RemoveFromSelectedMissionsInternal(const EMissionType& MissionType, const uint8& MissionIndex);

	bool IsAlreadySelected(const EMissionType& MissionType, const uint8& MissionIndex) const;
	bool IsServer() const;

	void UnlockMissionInternal(FMissionBaseRow* MissionData);

#pragma endregion

#pragma region Variables

private:

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

#pragma endregion

#pragma region Getters / Setters
public:

	const TSet<FString>& GetAllSelectedMissionNames() const;

	const TSet<FMissionData>& GetMissionDataForUI() const;

	const FAggroTriggerMissionRow* GetAggroTriggerMissionData(const EAggroTriggerMission& Mission) const;
	const FInteractionMissionRow* GetInteractionMissionData(const EInteractionMission& Mission) const;
	const FItemCollectMissionRow* GetItemCollectMissionData(const EItemCollectMission& Mission) const;
	const FItemUseMissionRow* GetItemUseMissionData(const EItemUseMission& Mission) const;
	const FKillMonsterMissionRow* GetKillMonsterMissionData(const EKillMonsterMission& Mission) const;
#pragma endregion

};
