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
	if (CheckIfGameStateIsValid() == false)
	{
		LOGV(Warning, TEXT("GameState Is Invalid"));
		return;
	}

	Missions.Reset(MissionsFromUI.Num());

	for (const FMissionData& MissionFromUI : MissionsFromUI)
	{
		UMissionBase* NewMission = nullptr;
		const EMissionType MissionType = MissionFromUI.MissionType;
		const uint8 MissionIndex = MissionFromUI.MissionIndex;

		switch (MissionType)
		{
		case EMissionType::AggroTrigger:
		{
			UAggroTriggerMission* NewAggroMission = NewObject<UAggroTriggerMission>(this);
			const FAggroTriggerMissionRow* AggroMissionData = GetAggroTriggerMissionData(EAggroTriggerMission(MissionIndex));
			FAggroMissionInitParams AggroMissionParams
			(
				AggroMissionData->MissionType,
				AggroMissionData->GoalCount,
				AggroMissionData->ConditionType,
				AggroMissionData->MissionName,
				AggroMissionData->MissionDescription,
				AggroMissionData->ExtraValues,
				AggroMissionData->bShouldCompleteInstantly
			);

			NewAggroMission->InitMission(AggroMissionParams, (EAggroTriggerMission)MissionIndex);
			NewMission = CastChecked<UMissionBase>(NewAggroMission);
			break;
		}
		case EMissionType::KillMonster:
		{
			UKillMonsterMission* NewKillMission = NewObject<UKillMonsterMission>(this);
			const FKillMonsterMissionRow* KillMissionData = GetKillMonsterMissionData(EKillMonsterMission(MissionIndex));
			FKillMissionInitParams KillMissionParams
			(
				KillMissionData->MissionType,
				KillMissionData->GoalCount,
				KillMissionData->ConditionType,
				KillMissionData->MissionName,
				KillMissionData->MissionDescription,
				KillMissionData->ExtraValues,
				KillMissionData->bShouldCompleteInstantly,
				KillMissionData->UnitId,
				KillMissionData->NeededSimultaneousKillCount,
				KillMissionData->KillInterval
			);

			NewKillMission->InitMission(KillMissionParams,(EKillMonsterMission)MissionIndex);
			NewMission = CastChecked<UMissionBase>(NewKillMission);
			break;
		}
		case EMissionType::Interaction:
		{
			UInteractionMission* NewInteractionMission = NewObject<UInteractionMission>(this);
			const FInteractionMissionRow* InteractionMissionData = GetInteractionMissionData(EInteractionMission(MissionIndex));
			FInteractiontMissionInitParams InteractionMissionParams
			(
				InteractionMissionData->MissionType,
				InteractionMissionData->GoalCount,
				InteractionMissionData->ConditionType,
				InteractionMissionData->MissionName,
				InteractionMissionData->MissionDescription,
				InteractionMissionData->ExtraValues,
				InteractionMissionData->bShouldCompleteInstantly
			);

			NewInteractionMission->InitMission(InteractionMissionParams, EInteractionMission(MissionIndex));
			NewMission = CastChecked<UMissionBase>(NewInteractionMission);
			break;
		}
		case EMissionType::ItemCollection:
		{
			UItemCollectionMission* NewItemCollectionMission = NewObject<UItemCollectionMission>(this);
			const FItemCollectMissionRow* ItemCollectionMissionData = GetItemCollectMissionData(EItemCollectMission(MissionIndex));
			FItemCollectMissionInitParams ItemCollectionMissionParams
			(
				ItemCollectionMissionData->MissionType,
				ItemCollectionMissionData->GoalCount,
				ItemCollectionMissionData->ConditionType,
				ItemCollectionMissionData->MissionName,
				ItemCollectionMissionData->MissionDescription,
				ItemCollectionMissionData->ExtraValues,
				ItemCollectionMissionData->bShouldCompleteInstantly,
				ItemCollectionMissionData->ItemId,
				ItemCollectionMissionData->bIsOreMission
			);

			NewItemCollectionMission->InitMission(ItemCollectionMissionParams, EItemCollectMission(MissionIndex));
			NewMission = CastChecked<UMissionBase>(NewItemCollectionMission);
			break;
		}
		case EMissionType::ItemUse:
		{
			UItemUseMission* NewItemUseMission = NewObject<UItemUseMission>(this);
			const FItemUseMissionRow* ItemUseMissionData = GetItemUseMissionData(EItemUseMission(MissionIndex));
			FItemUseMissionInitParams ItemUseMissionParams
			(
				ItemUseMissionData->MissionType,
				ItemUseMissionData->GoalCount,
				ItemUseMissionData->ConditionType,
				ItemUseMissionData->MissionName,
				ItemUseMissionData->MissionDescription,
				ItemUseMissionData->ExtraValues,
				ItemUseMissionData->bShouldCompleteInstantly
			);

			NewItemUseMission->InitMission(ItemUseMissionParams, EItemUseMission(MissionIndex));
			NewMission = CastChecked<UMissionBase>(NewItemUseMission);
			break;
		}
		default:
			check(false);
			return;
		}

		NewMission->OnCompleteMissionDelegate.BindUObject(this, &UMissionSubsystem::OnMissionComplete);
		Missions.Add(NewMission);
	}

	InGameState->RefreshActivatedMissionList();
}

void UMissionSubsystem::RequestBinding(UObject* Requester)
{
	for (TObjectPtr<UMissionBase> Mission : Missions)
	{
		Mission->BindDelegates(Requester);
	}
}

void UMissionSubsystem::RequestUnbinding(UObject* Requester)
{
	for (TObjectPtr<UMissionBase> Mission : Missions)
	{
		Mission->UnbindDelegates(Requester);
	}
}

void UMissionSubsystem::RemoveAllMissions()
{
	Missions.Reset();
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

bool UMissionSubsystem::CheckIfGameStateIsValid()
{
	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS == nullptr)
	{
		return false;
	}

	InGameState = GS;

	return !!InGameState;
}

void UMissionSubsystem::OnMissionComplete(const EMissionType& InMissionType, const uint8& InMissionIndex)
{
	if (CheckIfGameStateIsValid() == false)
	{
		LOGV(Error, TEXT("Invalid GS"));
		return;
	}

	// 완료시... 이벤트
	const FMissionBaseRow* MissionData = GetMissionData(InMissionType, InMissionIndex);
	
	if (MissionData->bShouldCompleteInstantly)
	{
		LOGV(Warning, TEXT("MissionComplete Instantly, Type : %d, MissionIndex : %d"), InMissionType, InMissionIndex);

	}
	else
	{
		LOGV(Warning, TEXT("Mission Condidion Met, Type : %d, MissionIndex : %d"), InMissionType, InMissionIndex);
	}

	InGameState->RefreshActivatedMissionList();
}

const TArray<FMissionData>& UMissionSubsystem::GetMissionDataForUI() const
{
	return MissionDataForUI;
}

const TArray<TObjectPtr<UMissionBase>>& UMissionSubsystem::GetActivatedMissions() const
{
	return Missions;
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
