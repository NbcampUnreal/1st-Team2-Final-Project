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
#include "UI/SelectedMissionListWidget.h"
#include "Framework/ADCampGameMode.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADGameInstance.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "UI/MissionsOnHUDWidget.h"
#include "TimerManager.h"
#include "EngineUtils.h"

#pragma region FastArraySerializer Methods

void FActivatedMissionInfo::PostReplicatedAdd(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	InArraySerializer.OnMissionInfosChangedDelegate.Broadcast(Index, InArraySerializer);
	LOGV(Warning, TEXT("Added, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

void FActivatedMissionInfo::PostReplicatedChange(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	InArraySerializer.OnMissionInfosChangedDelegate.Broadcast(Index, InArraySerializer);
	LOGV(Warning, TEXT("Changed, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

void FActivatedMissionInfo::PreReplicatedRemove(const FActivatedMissionInfoList& InArraySerializer)
{
	const TArray<FActivatedMissionInfo>& Infos = InArraySerializer.MissionInfoList;
	int32 Index = Infos.IndexOfByKey(*this);

	InArraySerializer.OnMissionInfosRemovedDelegate.Broadcast(Index, InArraySerializer);
	LOGV(Warning, TEXT("Removed, Index : %d, MissionType : %d, MissionIndex : %d, Value : %d"), Index, Infos[Index].MissionType, Infos[Index].MissionIndex, Infos[Index].CurrentProgress);
}

bool FActivatedMissionInfoList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FActivatedMissionInfo, FActivatedMissionInfoList>(MissionInfoList, DeltaParams, *this);
}

void FActivatedMissionInfoList::Add(const EMissionType& MissionType, const uint8& MissionIndex, bool bIsCompletedAlready = false)
{
	FActivatedMissionInfo NewInfo;
	NewInfo.MissionType = MissionType;
	NewInfo.MissionIndex = MissionIndex;
	NewInfo.CurrentProgress = 0;
	NewInfo.bIsCompleted = bIsCompletedAlready;
	
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

void FActivatedMissionInfoList::ModifyProgress(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress, bool bIsCompletedAlready = false)
{
	const int32 Index = Contains(MissionType, MissionIndex);
	if (Index == INDEX_NONE)
	{
		return;
	}

	MissionInfoList[Index].CurrentProgress = NewProgress;
	MissionInfoList[Index].bIsCompleted = bIsCompletedAlready;

	MarkItemDirty(MissionInfoList[Index]);
}

void FActivatedMissionInfoList::AddOrModify(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress, bool bIsCompletedAlready = false)
{
	if (Contains(MissionType, MissionIndex) == INDEX_NONE)
	{
		Add(MissionType, MissionIndex, bIsCompletedAlready);
	}

	ModifyProgress(MissionType, MissionIndex, NewProgress, bIsCompletedAlready);
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

#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	if (HasAuthority())
	{
		ReceiveDataFromGameInstance();

		RefreshActivatedMissionList();
	}

	const int32 SoundPoolInitCount = 10;
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Init(SoundPoolInitCount);
}

void AADInGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}

	if (GetClearCount() == 1)
	{
		FTimerHandle FirstClearTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			FirstClearTimerHandle,
			this,
			&AADInGameState::S_TriggerFirstClearUINotify,
			2.0f,
			false
		);
	}

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);

	StartPhaseUIAnim();
}

void AADInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADInGameState, SelectedLevelName);
	DOREPLIFETIME(AADInGameState, TeamCredits);
	DOREPLIFETIME(AADInGameState, CurrentPhase);
	DOREPLIFETIME(AADInGameState, CurrentDroneSeller);
	DOREPLIFETIME(AADInGameState, ActivatedMissionList);
	DOREPLIFETIME(AADInGameState, DestinationTarget);
}

void AADInGameState::PostNetInit()
{
	Super::PostNetInit();

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);

	StartPhaseUIAnim();
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
		ADGameInstance->ClearCount = ClearCount;
	}

	LOGVN(Error, TEXT("SelectedLevelName: %d / TeamCredits: %d"), SelectedLevelName, TeamCredits);
}

void AADInGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();

	UPlayerHUDComponent* HudComp = GetPlayerHudComponent();
	if (HudComp == nullptr)
	{
		LOGV(Error, TEXT("HudComp == nullptr"));
		return;
	}

	UMissionsOnHUDWidget* MissionsWidget = HudComp->GetMissionsOnHudWidget();
	if (MissionsWidget == nullptr)
	{
		LOGV(Error, TEXT("MissionsWidget == nullptr"));
		return;
	}

	MissionsWidget->InitWiget();
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

void AADInGameState::OnRep_CurrentDroneSeller()
{
	//AADPlayerState* PS = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AADPlayerState>();
	//if (PS == nullptr)
	//{
	//	LOGVN(Warning, TEXT("PS == nullptr"));
	//	return;
	//}

	//UADInventoryComponent* Inventory = PS->GetInventory();
	//if (Inventory == nullptr)
	//{
	//	LOGVN(Warning, TEXT("Inventory == nullptr"));
	//	return;
	//}

	//UToggleWidget* ToggleWidget = Inventory->GetToggleWidgetInstance();
	//if (IsValid(ToggleWidget) == false)
	//{
	//	LOGVN(Warning, TEXT("ToggleWidget is Invalid"));
	//	return;
	//}

	UPlayerHUDComponent* HudComp = GetPlayerHudComponent();
	if (HudComp == nullptr)
	{
		LOGV(Error, TEXT("HudComp == nullptr"));
		return;
	}

	UPlayerStatusWidget* PlayerStatusWidget = HudComp->GetPlayerStatusWidget();
	if (PlayerStatusWidget == nullptr)
	{
		LOGV(Error, TEXT("PlayerStatusWidget == nullptr"));
		return;
	}

	int32 TargetMoney = 0;
	int32 CurrentMoney = 0;
	float MoneyRatio = 0.f;

	if (CurrentDroneSeller)
	{
		TargetMoney = CurrentDroneSeller->GetTargetMoney();
		CurrentMoney = CurrentDroneSeller->GetCurrentMoney();
		MoneyRatio = CurrentDroneSeller->GetMoneyRatio();

		CurrentDroneSeller->OnCurrentMoneyChangedDelegate.RemoveAll(PlayerStatusWidget);
		CurrentDroneSeller->OnCurrentMoneyChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetDroneCurrentText);

		CurrentDroneSeller->OnTargetMoneyChangedDelegate.RemoveAll(PlayerStatusWidget);
		CurrentDroneSeller->OnTargetMoneyChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetDroneTargetText);

		CurrentDroneSeller->OnMoneyRatioChangedDelegate.RemoveAll(PlayerStatusWidget);
		CurrentDroneSeller->OnMoneyRatioChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetMoneyProgressBar);
	}

	PlayerStatusWidget->SetDroneTargetText(TargetMoney);
	PlayerStatusWidget->SetDroneCurrentText(CurrentMoney);
	PlayerStatusWidget->SetMoneyProgressBar(MoneyRatio);
}

void AADInGameState::OnRep_DestinationTarget()
{
	UPlayerHUDComponent* HudComp = GetPlayerHudComponent();
	if (HudComp == nullptr)
	{
		LOGV(Error, TEXT("HudComp == nullptr"));
		return;
	}

	UPlayerStatusWidget* PlayerStatusWidget = HudComp->GetPlayerStatusWidget();
	if (PlayerStatusWidget == nullptr)
	{
		LOGV(Error, TEXT("PlayerStatusWidget == nullptr"));
		return;
	}

	PlayerStatusWidget->SetCompassObject(DestinationTarget);
}

void AADInGameState::OnPlayerMinedCountChanged(AADPlayerState* PlayerState, int32 NewAmount)
{
	if (!HasAuthority()) return;

	if (NewAmount > TopMiningAmount)
	{
		TopMiner = PlayerState;
		TopMiningAmount = NewAmount;

		LOGV(Log, TEXT("New Top Miner: %s, Amount: %d"), *PlayerState->GetPlayerName(), NewAmount);
		M_BroadcastTopMinerChanged(TopMiner, TopMiningAmount);
	}
}

void AADInGameState::M_BroadcastTopMinerChanged_Implementation(AADPlayerState* NewTopMiner, int32 NewMiningAmount)
{
	TopMiner = NewTopMiner;
	TopMiningAmount = NewMiningAmount;
	OnTopMinerChangedDelegate.Broadcast(TopMiner, TopMiningAmount);

	LOGV(Log, TEXT("Broadcast Top Miner Changed: %s, Amount: %d"), *TopMiner->GetPlayerName(), TopMiningAmount);
}

void AADInGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (!HasAuthority()) return;

	if (AADPlayerState* ADPlayerState = Cast<AADPlayerState>(PlayerState))
	{
		LOGV(Log, TEXT("Register Ore Collected Value: %s"), *ADPlayerState->GetPlayerName());
		ADPlayerState->OnOreCollectedValueChangedDelegate.AddUObject(this, &AADInGameState::OnPlayerMinedCountChanged);

		int32 CurrentAmount = ADPlayerState->GetOreMinedCount();
		if (CurrentAmount > TopMiningAmount)
		{
			TopMiner = ADPlayerState;
			TopMiningAmount = CurrentAmount;
		}
	}
}

void AADInGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (!HasAuthority()) return;

	if (AADPlayerState* ADPlayerState = Cast<AADPlayerState>(PlayerState))
	{
		ADPlayerState->OnOreCollectedValueChangedDelegate.RemoveAll(this);

		// 중간에 최고 채굴자가 제거된 경우라도 업데이트를 일단 하지 않는다.
		// 추후, 접속이 종료된 Player가 있을 경우 여기서 처리한다.

		LOGV(Log, TEXT("Unregister Ore Collected Value: %s"), *ADPlayerState->GetPlayerName());
	}
}

void AADInGameState::ReceiveDataFromGameInstance()
{
	if (UADGameInstance* ADGameInstance = GetGameInstance<UADGameInstance>())
	{
		SelectedLevelName = ADGameInstance->SelectedLevelName;
		TeamCredits = ADGameInstance->TeamCredits;
		ClearCount = ADGameInstance->ClearCount;
	}
}

void AADInGameState::StartPhaseUIAnim()
{
	AADPlayerController* PC = GetWorld()->GetFirstPlayerController<AADPlayerController>();
	if (PC == nullptr)
	{
		LOGV(Error, TEXT("PC == nullptr"));
		return;
	}

	UPlayerHUDComponent* PlayerHudComp = PC->GetPlayerHUDComponent();
	if (PlayerHudComp == nullptr)
	{
		LOGV(Error, TEXT("PlayerHudComp == nullptr"));
		return;
	}

	if (SelectedLevelName == EMapName::test1 || SelectedLevelName == EMapName::test2 || SelectedLevelName == EMapName::SecondAbyss)
	{
		//이전 코드가 드론이 3개일 경우만 고려해서 드론의 개수에 유연한 리팩토링 필요 일단 급한대로 작성
		const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		if (CurrentMapName == "Submarine_Lobby") return;
		if (CurrentMapName == "SecondAbyss")
		{
			PlayerHudComp->SetMaxPhaseNumber(2);
		}
		else PlayerHudComp->SetMaxPhaseNumber(3);
		PlayerHudComp->PlayNextPhaseAnim(1);
		PlayerHudComp->SetCurrentPhaseOverlayVisible(true);
	}
	else
	{
		PlayerHudComp->SetCurrentPhaseOverlayVisible(false);
	}

	LOGVN(Error, TEXT("SelectedLevelName : %d"), SelectedLevelName);
}

void AADInGameState::SetDestinationTarget(AActor* NewDestinationTarget)
{
	if (HasAuthority() == false)
	{
		return;
	}

	DestinationTarget = NewDestinationTarget;
	OnRep_DestinationTarget();
}

UPlayerHUDComponent* AADInGameState::GetPlayerHudComponent()
{
	AADPlayerController* PC = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC == nullptr)
	{
		LOGV(Error, TEXT("PC == nullptr"));
		return nullptr;
	}

	UPlayerHUDComponent* HudComp = PC->GetPlayerHUDComponent();
	return HudComp;
}

int32 AADInGameState::GetClearCount() const
{
	return ClearCount;
}

void AADInGameState::SetClearCount(int32 NewClearCount)
{
	ClearCount = FMath::Max(NewClearCount, 0);
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

void AADInGameState::RefreshActivatedMissionList()
{
	if (HasAuthority() == false)
	{
		return;
	}

	const TArray<UMissionBase*>& Missions = GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetActivatedMissions();
	//ActivatedMissionList.Clear(Missions.Num());

	for (const UMissionBase* Mission : Missions)
	{
		ActivatedMissionList.AddOrModify(Mission->GetMissionType(), Mission->GetMissionIndex(), Mission->GetCurrentCount(), Mission->IsCompletedAlready());
	}

	OnMissionListRefreshedDelegate.Broadcast();
}

void AADInGameState::S_TriggerFirstClearUINotify()
{
	M_NotifyFirstClear();

	TWeakObjectPtr<AADInGameState> WeakThis(this);

	if (UADGameInstance* GI = GetGameInstance<UADGameInstance>())
	{
		GI->bHasPlayedTutorial = false;
	} 

	FTimerHandle LobbyTravelTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		LobbyTravelTimerHandle,
		FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				AADInGameState* StrongThis = WeakThis.Get();

				if (UWorld* World = StrongThis->GetWorld())
				{
					AADCampGameMode* GM = World->GetAuthGameMode<AADCampGameMode>();
					if (GM)
					{
						GM->TravelToMainLobby();
					}
					else
					{
						LOGV(Error, TEXT("Failed to get AADCampGameMode to travel to lobby."));
					}
				}
			}),
		5.0f,
		false
	);
}

void AADInGameState::M_NotifyFirstClear_Implementation()
{

	UPlayerHUDComponent* HudComp = GetPlayerHudComponent();
	if (HudComp)
	{
		HudComp->ShowFirstClearEndingWidget();
	}
	else
	{
		LOGV(Error, TEXT("M_NotifyFirstClear: PlayerHUDComponent를 찾을 수 없습니다."));
	}
}
