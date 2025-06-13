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

	void UnlockMission(const EAggroTriggerMission& Mission);
	void UnlockMission(const EInteractionMission& Mission);
	void UnlockMission(const EItemCollectMission& Mission);
	void UnlockMission(const EItemUseMission& Mission);
	void UnlockMission(const EKillMonsterMission& Mission);

	void ReceiveMissionDataFromUIData(const TArray<FMissionData>& Missions);

	void RequestBinding(UObject* Requester);
	void RequestUnbinding(UObject* Requester);

private:

	void MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData, const uint8& MissionIndex);

	bool IsServer() const;

	void UnlockMissionInternal(FMissionBaseRow* MissionsFromUI);

	bool CheckIfGameStateIsValid();

	void OnMissionComplete(const EMissionType& InMissionType, const uint8& InMissionIndex);

#pragma endregion

#pragma region Variables

private:

	UPROPERTY()
	TArray<TObjectPtr<UMissionBase>> Missions;

	TArray<FMissionData> MissionDataForUI;

	TArray<FAggroTriggerMissionRow*> AggroTriggerMissionDataArray;
	TArray<FInteractionMissionRow*> InteractionMissionDataArray;
	TArray<FItemCollectMissionRow*> ItemCollectMissionDataArray;
	TArray<FItemUseMissionRow*> ItemUseMissionDataArray;
	TArray<FKillMonsterMissionRow*> KillMonsterMissionDataArray;

	int32 AggroTriggerMissionCount = 0;
	int32 InteractionMissionCount = 0;
	int32 ItemCollectMissionCount = 0;
	int32 ItemUseMissionCount = 0;
	int32 KillMonsterMissionCount = 0;

	TObjectPtr<AADInGameState> InGameState;

#pragma endregion

#pragma region Getters / Setters
public:

	const TArray<FMissionData>& GetMissionDataForUI() const;
	const TArray<TObjectPtr<UMissionBase>>& GetActivatedMissions() const;

	const FAggroTriggerMissionRow* GetAggroTriggerMissionData(const EAggroTriggerMission& Mission) const;
	const FInteractionMissionRow* GetInteractionMissionData(const EInteractionMission& Mission) const;
	const FItemCollectMissionRow* GetItemCollectMissionData(const EItemCollectMission& Mission) const;
	const FItemUseMissionRow* GetItemUseMissionData(const EItemUseMission& Mission) const;
	const FKillMonsterMissionRow* GetKillMonsterMissionData(const EKillMonsterMission& Mission) const;

	const FMissionBaseRow* GetMissionData(const EMissionType& InMissionType, const uint8& InMissionIndex) const;

#pragma endregion

};
