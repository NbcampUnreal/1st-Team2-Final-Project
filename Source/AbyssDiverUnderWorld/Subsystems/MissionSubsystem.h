#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataRow/MissionDataRow/AggroTriggerMissionRow.h"
#include "DataRow/MissionDataRow/InteractionMissionRow.h"
#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"
#include "DataRow/MissionDataRow/ItemUseMissionRow.h"
#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"
#include "UI/MissionData.h"
#include "Missions/MissionBase.h"

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

	// BaseCamp UI(클라) → 서버 RPC에서 호출: 선택 결과를 서버 GI Subsystem에 저장
	void ReceiveMissionDataFromUIData(const TArray<FMissionData>& Missions);

	// 완료 미션 보관
	UFUNCTION(BlueprintCallable)
	void SetPendingCompletedMissions(const TArray<FCompletedMissionInfo>& InCompletedMissions);

	UFUNCTION(BlueprintCallable)
	void SetPendingSelectedMissions(const TArray<FMissionData>& InSelectedMissions);

	UFUNCTION(BlueprintCallable)
	const TArray<FCompletedMissionInfo>& GetPendingCompletedMissions() { return PendingCompletedMissions; }

	UFUNCTION(BlueprintCallable)
	const TArray<FMissionData>& GetPendingSelectedMissions() const { return PendingSelectedMissions; };

	UFUNCTION(BlueprintCallable)
	void ClearPendingSelectedMissions();
	UFUNCTION(BlueprintCallable)
	void ClearPendingCompletedMissions();

	// 한 번에 초기화가 필요할 때(선택/완료 전부)
	UFUNCTION(BlueprintCallable)
	void ResetAllPendingMissions();

private:

	void MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData, const uint8& MissionIndex);

	bool IsServer() const;

	void UnlockMissionInternal(FMissionBaseRow* MissionsFromUI);
#pragma endregion

#pragma region Variables

private:



	// SeamlessTravel 동안 서버에서만 들고 있는 1회용 팀 선택 결과
	TArray<FMissionData> PendingSelectedMissions;
	TArray<FCompletedMissionInfo> PendingCompletedMissions;

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

#pragma endregion

#pragma region Getters / Setters
public:

	const TArray<FMissionData>& GetMissionDataForUI() const;

	const FAggroTriggerMissionRow* GetAggroTriggerMissionData(const EAggroTriggerMission& Mission) const;
	const FInteractionMissionRow* GetInteractionMissionData(const EInteractionMission& Mission) const;
	const FItemCollectMissionRow* GetItemCollectMissionData(const EItemCollectMission& Mission) const;
	const FItemUseMissionRow* GetItemUseMissionData(const EItemUseMission& Mission) const;
	const FKillMonsterMissionRow* GetKillMonsterMissionData(const EKillMonsterMission& Mission) const;

	const FMissionBaseRow* GetMissionData(const EMissionType& InMissionType, const uint8& InMissionIndex) const;

#pragma endregion

};
