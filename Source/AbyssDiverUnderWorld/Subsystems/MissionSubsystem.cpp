#include "Subsystems/MissionSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"

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

	GI->AggroTriggerMissionTable->GetAllRows(TEXT("AggroTriggerMissionData"), AggroTriggerMissionData);

	Algo::Sort(AggroTriggerMissionData, [](const FAggroTriggerMissionRow* A, const FAggroTriggerMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	AggroTriggerMissionCount = AggroTriggerMissionData.Num();
	for (int32 i = 0; i < AggroTriggerMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)AggroTriggerMissionData[i]->Mission, TEXT("DT_AggroTriggerMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)AggroTriggerMissionData[i]);
	}

	GI->InteractionMissionTable->GetAllRows(TEXT("InteractionMission"), InteractionMissionData);

	Algo::Sort(InteractionMissionData, [](const FInteractionMissionRow* A, const FInteractionMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	InteractionMissionCount = InteractionMissionData.Num();
	for (int32 i = 0; i < InteractionMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)InteractionMissionData[i]->Mission, TEXT("DT_InteractionMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)InteractionMissionData[i]);

	}
	GI->ItemCollectMissionTable->GetAllRows(TEXT("ItemCollectMission"), ItemCollectMissionData);

	Algo::Sort(ItemCollectMissionData, [](const FItemCollectMissionRow* A, const FItemCollectMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	ItemCollectMissionCount = ItemCollectMissionData.Num();
	for (int32 i = 0; i < ItemCollectMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)ItemCollectMissionData[i]->Mission, TEXT("DT_ItemCollectMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)ItemCollectMissionData[i]);
	}

	GI->ItemUseMissionTable->GetAllRows(TEXT("ItemUseMission"), ItemUseMissionData);

	Algo::Sort(ItemUseMissionData, [](const FItemUseMissionRow* A, const FItemUseMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	ItemUseMissionCount = ItemUseMissionData.Num();
	for (int32 i = 0; i < ItemUseMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)ItemUseMissionData[i]->Mission, TEXT("DT_ItemUseMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}

		MakeAndAddMissionDataForUI((FMissionBaseRow*)ItemUseMissionData[i]);
	}

	GI->KillMonsterMissionTable->GetAllRows(TEXT("KillMonsterMission"), KillMonsterMissionData);

	Algo::Sort(KillMonsterMissionData, [](const FKillMonsterMissionRow* A, const FKillMonsterMissionRow* B)
		{
			return A->Mission < B->Mission;
		});

	KillMonsterMissionCount = KillMonsterMissionData.Num();
	for (int32 i = 0; i < KillMonsterMissionCount; ++i)
	{
		if (ensureMsgf(i == (int32)KillMonsterMissionData[i]->Mission, TEXT("DT_KillMonsterMissions에 %d번에 해당하는 데이터가 없습니다. 반드시 0번 Enum부터 채워주세요. "), i) == false)
		{
			return;
		}
	
		MakeAndAddMissionDataForUI((FMissionBaseRow*)KillMonsterMissionData[i]);
	}
}

void UMissionSubsystem::AddToSelectedMissions(const EAggroTriggerMission& Mission)
{
	if (AddToSelectedMissionInternal(EMissionType::AggroTrigger, (uint8)Mission) == false)
	{
		return;
	}

	const FAggroTriggerMissionRow* Data = GetAggroTriggerMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EInteractionMission& Mission)
{
	if (AddToSelectedMissionInternal(EMissionType::Interaction, (uint8)Mission) == false) 
	{
		return;
	}

	const FInteractionMissionRow* Data = GetInteractionMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EItemCollectMission& Mission)
{
	if(AddToSelectedMissionInternal(EMissionType::ItemCollection, (uint8)Mission) == false)
	{
		return;
	}

	const FItemCollectMissionRow* Data = GetItemCollectMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EItemUseMission& Mission)
{
	if(AddToSelectedMissionInternal(EMissionType::ItemUse, (uint8)Mission) == false)
	{
		return;
	}

	const FItemUseMissionRow* Data = GetItemUseMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::AddToSelectedMissions(const EKillMonsterMission& Mission)
{
	if(AddToSelectedMissionInternal(EMissionType::KillMonster, (uint8)Mission) == false)
	{
		return;
	}

	const FKillMonsterMissionRow* Data = GetKillMonsterMissionData(Mission);
	SelecedMissionNames.Add(Data->MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EAggroTriggerMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::AggroTrigger, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = AggroTriggerMissionData[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EInteractionMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::Interaction, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = InteractionMissionData[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EItemCollectMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::ItemCollection, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = ItemCollectMissionData[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EItemUseMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::ItemUse, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = ItemUseMissionData[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::RemoveFromSelectedMissions(const EKillMonsterMission& Mission)
{
	if(RemoveFromSelectedMissionsInternal(EMissionType::KillMonster, (uint8)Mission) == false)
	{
		return;
	}

	const FString& MissionName = KillMonsterMissionData[int32(Mission)]->MissionName;
	SelecedMissionNames.Remove(MissionName);
}

void UMissionSubsystem::UnlockMission(const EAggroTriggerMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)AggroTriggerMissionData[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EInteractionMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)InteractionMissionData[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EItemCollectMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)ItemCollectMissionData[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EItemUseMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)ItemUseMissionData[int32(Mission)]);
}

void UMissionSubsystem::UnlockMission(const EKillMonsterMission& Mission)
{
	UnlockMissionInternal((FMissionBaseRow*)KillMonsterMissionData[int32(Mission)]);
}

bool UMissionSubsystem::AddToSelectedMissionInternal(const EMissionType& MissionType, const uint8& MissionIndex)
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

void UMissionSubsystem::MakeAndAddMissionDataForUI(const FMissionBaseRow* MissionBaseData)
{
	FMissionData NewData;
	NewData.Title = MissionBaseData->MissionName;
	NewData.UnlockHint = MissionBaseData->UnlockHint;
	NewData.Stage = MissionBaseData->Stage;
	NewData.bIsUnlocked = (MissionBaseData->bIsLocked == false);
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

const TSet<FString>& UMissionSubsystem::GetAllSelectedMissionNames() const
{
	return SelecedMissionNames;
}

const TSet<FMissionData>& UMissionSubsystem::GetMissionDataForUI() const
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

	return AggroTriggerMissionData[int32(Mission)];
}

const FInteractionMissionRow* UMissionSubsystem::GetInteractionMissionData(const EInteractionMission& Mission) const
{
	if (InteractionMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return InteractionMissionData[int32(Mission)];
}

const FItemCollectMissionRow* UMissionSubsystem::GetItemCollectMissionData(const EItemCollectMission& Mission) const
{
	if (ItemCollectMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return ItemCollectMissionData[int32(Mission)];
}

const FItemUseMissionRow* UMissionSubsystem::GetItemUseMissionData(const EItemUseMission& Mission) const
{
	if (ItemUseMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return ItemUseMissionData[int32(Mission)];
}

const FKillMonsterMissionRow* UMissionSubsystem::GetKillMonsterMissionData(const EKillMonsterMission& Mission) const
{
	if (KillMonsterMissionCount <= int32(Mission))
	{
		LOGV(Error, TEXT("Invalid Mission"));
		return nullptr;
	}

	return KillMonsterMissionData[int32(Mission)];
}
