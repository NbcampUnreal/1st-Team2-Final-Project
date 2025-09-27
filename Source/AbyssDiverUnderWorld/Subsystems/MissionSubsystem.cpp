#include "Subsystems/MissionSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADInGameState.h"

#include "Missions/AggroTriggerMission.h"
#include "Missions/InteractionMission.h"
#include "Missions/ItemCollectionMission.h"
#include "Missions/ItemUseMission.h"
#include "Missions/KillMonsterMission.h"

#include "Kismet/GameplayStatics.h"

void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance());
	if (ensureMsgf(GI, TEXT("GameInstance가 유효하지 않음, ADGameInstance를 상속받은 것인지 확인 바람.")) == false)
	{
		return;
	}

	if (ensureMsgf(GI->AggroTriggerMissionTable, TEXT("AggroTriggerMissionTable이 유효하지 않음, GameInstance에서 할당해주었는지 확인 바람.")) == false)
	{
		return;
	}

	if (ensureMsgf(GI->InteractionMissionTable, TEXT("InteractionMissionTable이 유효하지 않음, GameInstance에서 할당해주었는지 확인 바람.")) == false)
	{
		return;
	}

	if (ensureMsgf(GI->ItemCollectMissionTable, TEXT("ItemCollectMissionTable이 유효하지 않음, GameInstance에서 할당해주었는지 확인 바람.")) == false)
	{
		return;
	}

	if (ensureMsgf(GI->ItemUseMissionTable, TEXT("ItemUseMissionTable이 유효하지 않음, GameInstance에서 할당해주었는지 확인 바람.")) == false)
	{
		return;
	}

	if (ensureMsgf(GI->KillMonsterMissionTable, TEXT("KillMonsterMissionTable이 유효하지 않음, GameInstance에서 할당해주었는지 확인 바람.")) == false)
	{
		return;
	}

	GI->AggroTriggerMissionTable->GetAllRows(TEXT("AggroTriggerMissionData"), AggroTriggerMissionDataArray);

	Algo::Sort(AggroTriggerMissionDataArray, [](const FAggroTriggerMissionRow* A, const FAggroTriggerMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	AggroTriggerMissionCount = AggroTriggerMissionDataArray.Num();
	for (int32 i = 0; i < AggroTriggerMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)AggroTriggerMissionDataArray[i]->Mission, TEXT("DT_AggroTriggerMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)AggroTriggerMissionDataArray[i], i);
	}

	GI->InteractionMissionTable->GetAllRows(TEXT("InteractionMission"), InteractionMissionDataArray);

	Algo::Sort(InteractionMissionDataArray, [](const FInteractionMissionRow* A, const FInteractionMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	InteractionMissionCount = InteractionMissionDataArray.Num();
	for (int32 i = 0; i < InteractionMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)InteractionMissionDataArray[i]->Mission, TEXT("DT_InteractionMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)InteractionMissionDataArray[i], i);

	}
	GI->ItemCollectMissionTable->GetAllRows(TEXT("ItemCollectMission"), ItemCollectMissionDataArray);

	Algo::Sort(ItemCollectMissionDataArray, [](const FItemCollectMissionRow* A, const FItemCollectMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	ItemCollectMissionCount = ItemCollectMissionDataArray.Num();
	for (int32 i = 0; i < ItemCollectMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)ItemCollectMissionDataArray[i]->Mission, TEXT("DT_ItemCollectMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)ItemCollectMissionDataArray[i], i);
	}

	GI->ItemUseMissionTable->GetAllRows(TEXT("ItemUseMission"), ItemUseMissionDataArray);

	Algo::Sort(ItemUseMissionDataArray, [](const FItemUseMissionRow* A, const FItemUseMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	ItemUseMissionCount = ItemUseMissionDataArray.Num();
	for (int32 i = 0; i < ItemUseMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)ItemUseMissionDataArray[i]->Mission, TEXT("DT_ItemUseMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)ItemUseMissionDataArray[i], i);
	}

	GI->KillMonsterMissionTable->GetAllRows(TEXT("KillMonsterMission"), KillMonsterMissionDataArray);

	Algo::Sort(KillMonsterMissionDataArray, [](const FKillMonsterMissionRow* A, const FKillMonsterMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	KillMonsterMissionCount = KillMonsterMissionDataArray.Num();
	for (int32 i = 0; i < KillMonsterMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)KillMonsterMissionDataArray[i]->Mission, TEXT("DT_KillMonsterMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}
	
		MakeAndAddMissionDataForUI((FMissionBaseRow*)KillMonsterMissionDataArray[i], i);
	}
}

void UMissionSubsystem::UnlockMission(const EAggroTriggerMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)AggroTriggerMissionDataArray[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EInteractionMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)InteractionMissionDataArray[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EItemCollectMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)ItemCollectMissionDataArray[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EItemUseMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)ItemUseMissionDataArray[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EKillMonsterMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)KillMonsterMissionDataArray[int32(Mission)]);
}

void UMissionSubsystem::ReceiveMissionDataFromUIData(const TArray<FMissionData>& MissionsFromUI)
{
	if (!IsServer())
		return;

	TArray<FMissionData> Filtered;
	Filtered.Reserve(MissionsFromUI.Num());
	for (const FMissionData& Data : MissionsFromUI)
	{
		if (GetMissionData(Data.MissionType, Data.MissionIndex))
		{
			Filtered.Add(Data);
		}
	}
	PendingSelectedMissions = MoveTemp(Filtered);

}

void UMissionSubsystem::SetPendingCompletedMissions(const TArray<FCompletedMissionInfo>& InCompletedMissions)
{
	if (!IsServer())
		return;

	PendingCompletedMissions = InCompletedMissions;
}

void UMissionSubsystem::SetPendingSelectedMissions(const TArray<FMissionData>& InSelectedMissions)
{
	if (!IsServer())
		return;

	PendingSelectedMissions = InSelectedMissions;
}

void UMissionSubsystem::ClearPendingSelectedMissions()
{
	if (!IsServer())
		return;

	PendingSelectedMissions.Reset();
}

void UMissionSubsystem::ClearPendingCompletedMissions()
{
	if (!IsServer())
		return;

	PendingCompletedMissions.Reset();
}

void UMissionSubsystem::ResetAllPendingMissions()
{
	PendingSelectedMissions.Reset();
	PendingCompletedMissions.Reset();
}

void UMissionSubsystem::MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData, const uint8& MissionIndex)
{
	FMissionData NewData;
	NewData.Title = MissionBaseData->MissionName;
	NewData.UnlockHint = MissionBaseData->UnlockHint;
	NewData.Stage = MissionBaseData->Stage;
	NewData.bIsUnlocked = (MissionBaseData->bIsLocked == false);
	NewData.MissionType = MissionBaseData->MissionType;
	NewData.MissionIndex = MissionIndex;

	MissionDataForUI.Add(NewData);
}

bool UMissionSubsystem::IsServer() const
{
	return !!GetWorld() && (GetWorld()->GetNetMode() != ENetMode::NM_Client);
}

void UMissionSubsystem::UnlockMissionInternal(FMissionBaseRow* MissionData)
{
	MissionData->bIsLocked = false;
}

const TArray<FMissionData>& UMissionSubsystem::GetMissionDataForUI() const
{
	return MissionDataForUI;
}


const FAggroTriggerMissionRow* UMissionSubsystem::GetAggroTriggerMissionData(const EAggroTriggerMission& Mission) const
{
	if (AggroTriggerMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return AggroTriggerMissionDataArray[int32(Mission)];
}

const FInteractionMissionRow* UMissionSubsystem::GetInteractionMissionData(const EInteractionMission& Mission) const
{
	if (InteractionMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return InteractionMissionDataArray[int32(Mission)];
}

const FItemCollectMissionRow* UMissionSubsystem::GetItemCollectMissionData(const EItemCollectMission& Mission) const
{
	if (ItemCollectMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return ItemCollectMissionDataArray[int32(Mission)];
}

const FItemUseMissionRow* UMissionSubsystem::GetItemUseMissionData(const EItemUseMission& Mission) const
{
	if (ItemUseMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return ItemUseMissionDataArray[int32(Mission)];
}

const FKillMonsterMissionRow* UMissionSubsystem::GetKillMonsterMissionData(const EKillMonsterMission& Mission) const
{
	if (KillMonsterMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return KillMonsterMissionDataArray[int32(Mission)];
}

const FMissionBaseRow* UMissionSubsystem::GetMissionData(const EMissionType& InMissionType, const uint8& InMissionIndex) const
{
	FMissionBaseRow* Data = nullptr;

	switch (InMissionType)
	{
	case EMissionType::AggroTrigger:
		Data = (FMissionBaseRow*)GetAggroTriggerMissionData(EAggroTriggerMission(InMissionIndex));
		break;
	case EMissionType::Interaction:
		Data = (FMissionBaseRow*)GetInteractionMissionData(EInteractionMission(InMissionIndex));
		break;
	case EMissionType::ItemCollection:
		Data = (FMissionBaseRow*)GetItemCollectMissionData(EItemCollectMission(InMissionIndex));
		break;
	case EMissionType::ItemUse:
		Data = (FMissionBaseRow*)GetItemUseMissionData(EItemUseMission(InMissionIndex));
		break;
	case EMissionType::KillMonster:
		Data = (FMissionBaseRow*)GetKillMonsterMissionData(EKillMonsterMission(InMissionIndex));
		break;
	default:
		check(false);
		return nullptr;
	}

	return Data;
}
