#include "Framework/ADInGameState.h"
#include "ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/MissionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "DataRow/PhaseGoalRow.h"
#include "ADPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/ADInventoryComponent.h"
#include "Interactable/OtherActors/ADDroneSeller.h"
#include "UI/ToggleWidget.h"
#include "Missions/MissionBase.h"

#pragma region FastArraySerializer Methods

void FActivatedMissionInfo::PostReplicatedAdd(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	LOGV(Warning, TEXT("Added, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

void FActivatedMissionInfo::PostReplicatedChange(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	LOGV(Warning, TEXT("Changed, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

void FActivatedMissionInfo::PreReplicatedRemove(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	LOGV(Warning, TEXT("Removed, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

bool FActivatedMissionInfoList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FActivatedMissionInfo, FActivatedMissionInfoList>(MissionInfoList, DeltaParams, *this);
}

void FActivatedMissionInfoList::Add(const EMissionType& MissionType, const uint8& MissionIndex)
{
	FActivatedMissionInfo NewInfo;
	NewInfo.MissionType = MissionType;
	NewInfo.MissionIndex = MissionIndex;
	NewInfo.CurrentProgress = 0;
	NewInfo.bIsCompleted = false;

	MissionInfoList.Emplace(MoveTemp(NewInfo));
	MarkItemDirty(MissionInfoList.Last());
}

void FActivatedMissionInfoList::Remove(const EMissionType& MissionType, const uint8& MissionIndex)
{
	const int32 Index = Contains(MissionType, MissionIndex);
	if (Index == INDEX_NONE)
	{
		return;
	}

	MissionInfoList.RemoveAt(Index);
	MarkArrayDirty();
}

void FActivatedMissionInfoList::ModifyProgress(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress)
{
	const int32 Index = Contains(MissionType, MissionIndex);
	if (Index == INDEX_NONE)
	{
		return;
	}

	MissionInfoList[Index].CurrentProgress = NewProgress;
	MarkItemDirty(MissionInfoList[Index]);
}

void FActivatedMissionInfoList::AddOrModity(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress)
{
	if (Contains(MissionType, MissionIndex) == INDEX_NONE)
	{
		Add(MissionType, MissionIndex);
	}

	ModifyProgress(MissionType, MissionIndex, NewProgress);
}

int32 FActivatedMissionInfoList::Contains(const EMissionType& MissionType, const uint8& MissionIndex)
{
	const int32 InfoCount = MissionInfoList.Num();
	for (int32 i = 0; i < InfoCount; ++i)
	{
		if (MissionInfoList[i].MissionType == MissionType && MissionInfoList[i].MissionIndex == MissionIndex)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void FActivatedMissionInfoList::Clear(const int32 SlackCount)
{
	MissionInfoList.Empty(SlackCount);
	MarkArrayDirty();
}

#pragma endregion

AADInGameState::AADInGameState()
	: SelectedLevelName(EMapName::Max)
	, TeamCredits(0)
	, CurrentPhase(1)
	, MaxPhase(3)
{
	bReplicates = true;
	
}

void AADInGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		ReceiveDataFromGameInstance();
	}
}

void AADInGameState::BeginPlay()
{
	Super::BeginPlay();

	const int32 SoundPoolInitCount = 10;
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Init(SoundPoolInitCount);

	if (HasAuthority() == false)
	{
		return;
	}

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);

	const TArray<UMissionBase*>& Missions = GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetActivatedMissions();
	ActivatedMissionList.Clear(Missions.Num());

	for (const UMissionBase* Mission : Missions)
	{
		ActivatedMissionList.Add(Mission->GetMissionType(), Mission->GetMissionIndex());
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [&]()
		{
			const TArray<UMissionBase*>& Missions2 = GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetActivatedMissions();
			for (const UMissionBase* Mission : Missions2)
			{
				static int32 i = 0;
				ActivatedMissionList.ModifyProgress(Mission->GetMissionType(), Mission->GetMissionIndex(), ++i);
			}

		}, 5, false);

	FTimerHandle TimerHandle2;
	GetWorldTimerManager().SetTimer(TimerHandle2, [&]()
		{
			ActivatedMissionList.Clear();

		}, 10, false);
	
}

void AADInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADInGameState, SelectedLevelName);
	DOREPLIFETIME(AADInGameState, TeamCredits);
	DOREPLIFETIME(AADInGameState, CurrentPhase);
	DOREPLIFETIME(AADInGameState, CurrentDroneSeller);
	DOREPLIFETIME(AADInGameState, ActivatedMissionList);
}

void AADInGameState::PostNetInit()
{
	Super::PostNetInit();

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);
}

void AADInGameState::AddTeamCredit(int32 Credit)
{
	if (!HasAuthority()) return;

	TeamCredits += Credit;
	OnRep_Money();
}

void AADInGameState::IncrementPhase()
{
	if (!HasAuthority()) return;

	CurrentPhase = FMath::Clamp(CurrentPhase + 1, 0, MaxPhase);
	OnRep_Phase();
}

void AADInGameState::SendDataToGameInstance()
{
	if (UADGameInstance* ADGameInstance = GetGameInstance<UADGameInstance>())
	{
		ADGameInstance->SelectedLevelName = SelectedLevelName;
		ADGameInstance->TeamCredits = TeamCredits;
	}

	LOGVN(Error, TEXT("SelectedLevelName: %d / TeamCredits: %d"), SelectedLevelName, TeamCredits);
}

void AADInGameState::OnRep_Money()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("TotalTeamCredit updated: %d"), TeamCredits);
	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
}

void AADInGameState::OnRep_Phase()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("Phase updated: %d/%d"), CurrentPhase, MaxPhase);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
}

void AADInGameState::OnRep_PhaseGoal()
{
	LOGVN(Error, TEXT("PhaseGoal updated: %d"), CurrentPhaseGoal);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);
}

void AADInGameState::OnRep_CurrentDroneSeller()
{
	AADPlayerState* PS = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGVN(Warning, TEXT("PS == nullptr"));
		return;
	}

	UADInventoryComponent* Inventory = PS->GetInventory();
	if (Inventory == nullptr)
	{
		LOGVN(Warning, TEXT("Inventory == nullptr"));
		return;
	}

	UToggleWidget* ToggleWidget = Inventory->GetToggleWidgetInstance();
	if (IsValid(ToggleWidget) == false)
	{
		LOGVN(Warning, TEXT("ToggleWidget is Invalid"));
		return;
	}

	ToggleWidget->SetDroneTargetText(CurrentDroneSeller->GetTargetMoney());
	ToggleWidget->SetDroneCurrentText(CurrentDroneSeller->GetCurrentMoney());

	CurrentDroneSeller->OnCurrentMoneyChangedDelegate.RemoveAll(ToggleWidget);
	CurrentDroneSeller->OnCurrentMoneyChangedDelegate.AddUObject(ToggleWidget, &UToggleWidget::SetDroneCurrentText);

	CurrentDroneSeller->OnTargetMoneyChangedDelegate.RemoveAll(ToggleWidget);
	CurrentDroneSeller->OnTargetMoneyChangedDelegate.AddUObject(ToggleWidget, &UToggleWidget::SetDroneTargetText);
}

void AADInGameState::ReceiveDataFromGameInstance()
{
	if (UADGameInstance* ADGameInstance = GetGameInstance<UADGameInstance>())
	{
		SelectedLevelName = ADGameInstance->SelectedLevelName;
		TeamCredits = ADGameInstance->TeamCredits;
		if (const FPhaseGoalRow* GoalData = ADGameInstance->GetSubsystem<UDataTableSubsystem>()->GetPhaseGoalData(SelectedLevelName, CurrentPhase))
		{
			CurrentPhaseGoal = GoalData->GoalCredit;
		}
	}

	LOGVN(Error, TEXT("SelectedLevelName: %d / TeamCredits: %d / CurrentPhaseGoal : %d"), SelectedLevelName, TeamCredits, CurrentPhaseGoal);
}

FString AADInGameState::GetMapDisplayName() const
{
	const UEnum* EnumPtr = StaticEnum<EMapName>();
	if (EnumPtr == nullptr)
	{
		return TEXT("INVALID");
	}

	FString DisplayName = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SelectedLevelName)).ToString();
	LOGV(Warning, TEXT("Display Name : %s"), *DisplayName);

	return DisplayName;

}
