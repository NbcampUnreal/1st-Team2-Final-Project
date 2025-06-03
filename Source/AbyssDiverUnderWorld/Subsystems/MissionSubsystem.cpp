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
	LOGV(Log, TEXT("???????????????"));
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

void UMissionSubsystem::AddToSelectedMissions(const EAggroTriggerMission& Mission)
{
	if (AddToSelectedMissionFromIndex(EMissionType::AggroTrigger, (uint8)Mission) == false)
	{
		return;
	}

	const FAggroTriggerMissionRow* Data = GetAggroTriggerMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EInteractionMission& Mission)
{
	if (AddToSelectedMissionFromIndex(EMissionType::Interaction, (uint8)Mission) == false) 
	{
		return;
	}

	const FInteractionMissionRow* Data = GetInteractionMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EItemCollectMission& Mission)
{
	if(AddToSelectedMissionFromIndex(EMissionType::ItemCollection, (uint8)Mission) == false)
	{
		return;
	}

	const FItemCollectMissionRow* Data = GetItemCollectMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EItemUseMission& Mission)
{
	if(AddToSelectedMissionFromIndex(EMissionType::ItemUse, (uint8)Mission) == false)
	{
		return;
	}

	const FItemUseMissionRow* Data = GetItemUseMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EKillMonsterMission& Mission)
{
	if(AddToSelectedMissionFromIndex(EMissionType::KillMonster, (uint8)Mission) == false)
	{
		return;
	}

	const FKillMonsterMissionRow* Data = GetKillMonsterMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

bool UMissionSubsystem::AddToSelectedMissionFromIndex(const EMissionType& MissionType, const uint8& MissionIndex)
{
	if (SelectedMissions.Num() <= (int32)MissionType)
	{
		LOGV(Error, TEXT("Invalid MissionType : %d"), MissionType);
		return false;
	}
	if (IsAlreadySelected(MissionType, MissionIndex))
	{
		return false;
	}

	SelectedMissions[MissionType].Add(MissionIndex);
	return true;
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EAggroTriggerMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::AggroTrigger, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = AggroTriggerMissionDataArray[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EInteractionMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::Interaction, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = InteractionMissionDataArray[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EItemCollectMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::ItemCollection, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = ItemCollectMissionDataArray[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EItemUseMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::ItemUse, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = ItemUseMissionDataArray[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EKillMonsterMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::KillMonster, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = KillMonsterMissionDataArray[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
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

void UMissionSubsystem::ClearSelectedMissions(const int32& SlackCount)
{
	SelectedMissions.Empty(SlackCount);
}

void UMissionSubsystem::ReceiveMissionDataFromUIData(const TArray<FMissionData>& MissionsFromUI)
{
	if (CheckIfGameStateIsValid() == false)
	{
		LOGV(Warning, TEXT("GameState Is Invalid"));
		return;
	}

	Missions.Empty(MissionsFromUI.Num());

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
				AggroMissionData->ExtraValues
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
				KillMissionData->UnitId
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
				InteractionMissionData->ExtraValues
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
				ItemCollectionMissionData->ExtraValues
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
			{
				ItemUseMissionData->MissionType,
				ItemUseMissionData->GoalCount,
				ItemUseMissionData->ConditionType,
				ItemUseMissionData->MissionName,
				ItemUseMissionData->MissionDescription,
				ItemUseMissionData->ExtraValues
			};

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

bool UMissionSubsystem::RemoveFromSelectedMissionsInternal(const EMissionType& MissionType, const uint8& MissionIndex)
{
	if (SelectedMissions.Num() <= (int32)MissionType)
	{
		LOGV(Error, TEXT("Invalid MissionType : %d"), MissionType);
		return false;
	}

	if (IsAlreadySelected(MissionType, MissionIndex) == false)
	{
		return false;
	}

	SelectedMissions[MissionType].Remove(MissionIndex);
	return true;
}

bool UMissionSubsystem::IsAlreadySelected(const EMissionType& MissionType, const uint8& MissionIndex) const
{
	if (SelectedMissions.Contains(MissionType) == false)
	{
		return false;
	}

	if (SelectedMissions[MissionType].Contains(MissionIndex) == false)
	{
		return false;
	}

	return true;
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
	if (IsValid(InGameState))
	{
		return true;
	}

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
	// 완료시... 이벤트
}

const TSet<FString>& UMissionSubsystem::GetAllSelectedMissionNames() const
{
	return SelecedMissionNames;
}

const TSet<FMissionData>& UMissionSubsystem::GetMissionDataForUI() const
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
