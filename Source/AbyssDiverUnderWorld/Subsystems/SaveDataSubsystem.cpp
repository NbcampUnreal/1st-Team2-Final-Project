#include "Subsystems/SaveDataSubsystem.h"

#include "AbyssDiverUnderWorld.h"

#include "SaveData/CoopData/CoopSessionSaveGame.h"
#include "SaveData/CoopData/SavedSessionInfoSaveGame.h"

#include "Framework/ADCampGameMode.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"

#include "Inventory/ADInventoryComponent.h"
#include "Character/UpgradeComponent.h"
#include "Subsystems/MissionSubsystem.h"

#include "Kismet/GameplayStatics.h"

const FString USaveDataSubsystem::SavedSessionInfoSlotName = TEXT("SavedSessionInfo");
const FString USaveDataSubsystem::SaveSlotNamePrefix = TEXT("SavedSession_");

void USaveDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &USaveDataSubsystem::OnWorldTearDown);
	AsyncLoadSavedGameListInfo();
}

void USaveDataSubsystem::AsyncSaveCurrentGame()
{
	if (IsServer() == false)
	{
		return;
	}

	const FDateTime Now = FDateTime::Now();
	const FString SaveGameName = Now.ToString(TEXT("[%Y-%m-%d %H:%M:%S] ")) + TEXT("Auto Saved");
	AsyncSaveCurrentGame(SaveGameName, false);
}

void USaveDataSubsystem::AsyncSaveCurrentGame(const FString& SaveGameName, bool bShouldOverwrite)
{
	if (IsServer() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	/*
	* 저장해야 할 것
	* PlayerNetId - UADGameInstance
	* Inventory - PlayerState
	* Upgrade - PlayerState
	* ClearCount - ADInGameState
	* TotalMoney - ADInGameState(TeamCredits)
	* SaveDataName - Param
	*/

	UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance());
	if (GI == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Save Game, GameInstance is not valid"));
		return;
	}

	AADInGameState* GS = World->GetGameState<AADInGameState>();
	if (GS == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Save Game, GameState is not valid"));
		return;
	}
	
	const TMap<FString, int32>& CurrentPlayerIdMap = GI->GetCurrentPlayerIdMap();
	const int32 PlayerCount = CurrentPlayerIdMap.Num();
	const int32 MaxPlayerCount = 4;
	if (PlayerCount > MaxPlayerCount)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Fail to Save Game, Weird Player Count : %d"), PlayerCount));
		return;
	}

	TArray<FPlayerProgressData> PlayerProgressDataList;

	for (const TPair<FString, int32>& NetIdPlayerIdPair : CurrentPlayerIdMap)
	{
		const int32 PlayerId = NetIdPlayerIdPair.Value;

		AADPlayerState* PS = Cast<AADPlayerState>(UGameplayStatics::GetPlayerState(World, PlayerId));
		if (PS == nullptr)
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Save Game, PlayerState is not valid, PlayerId : %d"), PlayerId));
			return;
		}

		UADInventoryComponent* Inventory = PS->GetInventory();
		if (IsValid(Inventory) == false || Inventory->IsBeingDestroyed())
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Save Game, InventoryComponent is not valid, PlayerId : %d"), PlayerId));
			return;
		}

		const FInventoryList& InventoryList = Inventory->GetInventoryList();
		
		UUpgradeComponent* UpgradeComponent = PS->GetUpgradeComp();
		if (IsValid(UpgradeComponent) == false || UpgradeComponent->IsBeingDestroyed())
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Save Game, UpgradeComponent is not valid, PlayerId : %d"), PlayerId));
			return;
		}

		FPlayerProgressData PlayerProgressData
		(
			NetIdPlayerIdPair.Key,
			InventoryList.Items,
			UpgradeComponent->GetUpgradeGradeMap()
		);

		PlayerProgressDataList.Emplace(PlayerProgressData);
	}

	FCoopSessionSaveData SessionData
	(
		PlayerProgressDataList,
		GS->GetClearCount(),
		GS->GetTotalTeamCredit(),
		SaveGameName
	);

	UCoopSessionSaveGame* SessionSaveGame = Cast<UCoopSessionSaveGame>(UGameplayStatics::CreateSaveGameObject(UCoopSessionSaveGame::StaticClass()));
	if (SessionSaveGame == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Save Game, SessionSaveGame is not valid"));
		return;
	}

	SessionSaveGame->SetSaveData(SessionData);

	const int32 SafetyNumber = 100;
	for (int32 i = 0; i < SafetyNumber; ++i)
	{
		int32 RandomNumber = FMath::Rand();
		FString NewSlotName = SaveSlotNamePrefix + FString::FromInt(RandomNumber);

		bIsSavingNow = true;

		if (bShouldOverwrite)
		{
			FAsyncSaveGameToSlotDelegate SaveDelegateWithOverwrite;
			SaveDelegateWithOverwrite.BindUObject(this, &USaveDataSubsystem::OnSaveSessionGame);
			UGameplayStatics::AsyncSaveGameToSlot(SessionSaveGame, NewSlotName, 0, SaveDelegateWithOverwrite);
			LastSavedSessionGame = SessionSaveGame;
			LastSavedGameSlotName = NewSlotName;

			return;
		}
		else if(IsExistSavedSessionGame(NewSlotName) == false)
		{
			FAsyncSaveGameToSlotDelegate SaveDelegateWithoutOverwrite;
			SaveDelegateWithoutOverwrite.BindUObject(this, &USaveDataSubsystem::OnSaveSessionGame);
			UGameplayStatics::AsyncSaveGameToSlot(SessionSaveGame, NewSlotName, 0, SaveDelegateWithoutOverwrite);
			LastSavedSessionGame = SessionSaveGame;
			LastSavedGameSlotName = NewSlotName;

			return;
		}
	}

	PrintLogWithScreen(TEXT("Fail to Save Session Game. Atteption exceeded."));
}

void USaveDataSubsystem::AsyncLoadLastSavedGame()
{
	if (IsServer() == false)
	{
		return;
	}

	if (LastSavedGameSlotName.IsEmpty())
	{
		PrintLogWithScreen(TEXT("LastSavedGame Is Not Exist"));
		return;
	}

	AsyncLoadSavedGame(LastSavedGameSlotName);
}

void USaveDataSubsystem::AsyncLoadSavedGame(const FString& SavedSlotName)
{
	if (IsServer() == false)
	{
		return;
	}

	AADCampGameMode* CampGameMode = Cast<AADCampGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (CampGameMode == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Load Saved Game Because of Not Valid CampGameMode"));
		return;
	}

	if (CampGameMode->HasPressedTravel())
	{
		PrintLogWithScreen(TEXT("Fail to Load Saved Game Because Game is Already Started"));
		return;
	}

	bIsLoadingNow = true;

	FAsyncLoadGameFromSlotDelegate OnLoadGameDelegate;
	OnLoadGameDelegate.BindUObject(this, &USaveDataSubsystem::OnLoadedSavedGame);
	UGameplayStatics::AsyncLoadGameFromSlot(SavedSlotName, 0, OnLoadGameDelegate);
}

void USaveDataSubsystem::AsyncSaveSavedGameListInfo()
{
	if (IsServer() == false)
	{
		return;
	}

	FAsyncSaveGameToSlotDelegate SaveDelegateWithoutOverwrite;
	SaveDelegateWithoutOverwrite.BindUObject(this, &USaveDataSubsystem::OnSaveSavedGameListInfoGame);
	UGameplayStatics::AsyncSaveGameToSlot(CachedSavedSessionInfo, SavedSessionInfoSlotName, 0, SaveDelegateWithoutOverwrite);
}

void USaveDataSubsystem::AsyncLoadSavedGameListInfo()
{
	if (IsServer() == false)
	{
		return;
	}

	bIsLoadingNow = true;

	FAsyncLoadGameFromSlotDelegate OnLoadSessionInfoDelegate;
	OnLoadSessionInfoDelegate.BindUObject(this, &USaveDataSubsystem::OnLoadedSavedGameListInfo);
	UGameplayStatics::AsyncLoadGameFromSlot(SavedSessionInfoSlotName, 0, OnLoadSessionInfoDelegate);
}

void USaveDataSubsystem::DeleteAllSavedGame()
{
	if (IsServer() == false)
	{
		return;
	}

	if (CachedSavedSessionInfo == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Delete All Saved Game"));
		return;
	}

	for (auto It = CachedSavedSessionInfo->SavedSessionInfos.CreateIterator(); It; ++It)
	{
		DeleteSavedGame(It->Key);
	}
}

void USaveDataSubsystem::DeleteSavedGame(const FString& SavedSlotName)
{
	if (IsServer() == false)
	{
		return;
	}

	if (CachedSavedSessionInfo == nullptr)
	{
		PrintLogWithScreen(TEXT("Fail to Delete Saved Game Because SavedSessionInfo is not valid"));
		return;
	}

	if (UGameplayStatics::DeleteGameInSlot(SavedSlotName, 0) == false)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Fail to Delete Saved Game, Slot Name : %s"), *SavedSlotName));
	}

	if (CachedSavedSessionInfo->SavedSessionInfos.Contains(SavedSlotName))
	{
		FString SaveFileName = CachedSavedSessionInfo->SavedSessionInfos[SavedSlotName].SaveName;
		CachedSavedSessionInfo->SavedSessionInfos.Remove(SavedSlotName);
		PrintLogWithScreen(FString::Printf(TEXT("%s(%s) is Deleted"), *SavedSlotName, *SaveFileName));
	}
	else
	{
		PrintLogWithScreen(FString::Printf(TEXT("%s is not cached"), *SavedSlotName));
	}
}

void USaveDataSubsystem::DisplayAllSaves()
{
	LOGV(Warning, TEXT("StartDisplay"));
	if (IsServer() == false)
	{
		return;
	}

	if (CachedSavedSessionInfo == nullptr)
	{
		return;
	}

	LOGV(Warning, TEXT("========From CachedInfo==========="));

	TArray<FString> Slots;
	for (const auto& Saved : CachedSavedSessionInfo->SavedSessionInfos)
	{
		LOGV(Warning, TEXT("Slot : %s, UnixTime : %d, SaveName : %s"), *Saved.Key, Saved.Value.SavedUnixTime, *Saved.Value.SaveName);
		Slots.Add(Saved.Key);
	}

	LOGV(Warning, TEXT("========End CachedInfo==========="));
	LOGV(Warning, TEXT("========From ActualSlot==========="));
	for (const auto& Slot : Slots)
	{
		bool bIsExist = UGameplayStatics::DoesSaveGameExist(Slot, 0);
		if (bIsExist)
		{
			UCoopSessionSaveGame* SavedGame = Cast<UCoopSessionSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
			LOGV(Warning, TEXT("---------Slot : %s, SavedName : %s-----------------"), *Slot, *SavedGame->GetSaveData().SaveDataName);
			LOGV(Warning, TEXT("ClearCount : %d, TotalMoney : %d"), SavedGame->GetSaveData().ClearCount, SavedGame->GetSaveData().TotalMoney);
			
			for (const auto& PlayerProgressData : SavedGame->GetSaveData().PlayerProgressDataList)
			{
				LOGV(Warning, TEXT("~~~~~Player %s Item Data~~~~~~~~~~"), *PlayerProgressData.PlayerNetId);
				for (const auto& Item : PlayerProgressData.ItemDataList)
				{
					LOGV(Warning, TEXT("Item Name : %s"), *Item.Name.ToString());
				}

				LOGV(Warning, TEXT("~~~~~Player %s Upgrade Data~~~~~~~~~~"), *PlayerProgressData.PlayerNetId);
				TArray<uint8> UpgradeInfos = PlayerProgressData.UpgradeGradeMap;
				for (int32 i = 0; i < UpgradeInfos.Num(); ++i)
				{
					LOGV(Warning, TEXT("UpgradeType :%d, Grade : %d"), i, UpgradeInfos[i]);
				}
			}
		}
		else
		{
			LOGV(Warning, TEXT("- Slot %s is not exist"), *Slot);
		}
	}

	LOGV(Warning, TEXT("========End ActualSlot==========="));
}

void USaveDataSubsystem::OnSaveSessionGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded)
{
	if (IsServer() == false)
	{
		return;
	}

	if (bIsSucceeded == false)
	{
		PrintLogWithScreen(TEXT("SessionGame Save Failed"));
		return;
	}

	// 호스트가 아니면 저장 불가
	if (UserIndex != 0)
	{
		PrintLogWithScreen(FString::Printf(TEXT("SessionGame Save Failed, Slot : %s, UserIndex : %d"), *SlotName, UserIndex));
		return;
	}

	CachedSavedSessionInfo->SavedSessionInfos.Add({ SlotName, FSavedSessionInfo(LastSavedSessionGame->GetSaveData().SaveDataName) });
	AsyncSaveSavedGameListInfo();

	// 저장 성공 알리기
	PrintLogWithScreen(FString::Printf(TEXT("SessionGame Save Succeeded, Slot : %s, UserIndex : %d"), *SlotName, UserIndex));
}

void USaveDataSubsystem::OnSaveSavedGameListInfoGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded)
{
	bIsSavingNow = false;

	if (IsServer() == false)
	{
		return;
	}

	if (bIsSucceeded == false)
	{
		PrintLogWithScreen(TEXT("Fail to Save SavedGameListInfo"));
		return;
	}

	// 호스트가 아니면 저장 불가
	if (UserIndex != 0)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Fail to Save SavedGameListInfo, Slot : %s, UserIndex : %d"), *SlotName, UserIndex));
		return;
	}

	// 저장 리스트 갱신 완료..
	PrintLogWithScreen(FString::Printf(TEXT("SavedGameListInfo Save Succeeded, Slot : %s, UserIndex : %d"), *SlotName, UserIndex));
}

void USaveDataSubsystem::OnLoadedSavedGame(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame)
{
	bIsLoadingNow = false;

	if (IsServer() == false)
	{
		return;
	}

	// 호스트가 아니면 불러오기 불가
	if (UserIndex != 0)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Load Failed With Weird UserIndex : %d"), UserIndex));
		return;
	}

	UCoopSessionSaveGame* SavedGame = Cast<UCoopSessionSaveGame>(LoadedSavedGame);
	if (SavedGame == nullptr)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Load Failed, Slot : %s, UserIndex : %d"), *LoadedSlotName, UserIndex));
		return;
	}

	const FCoopSessionSaveData& SavedData = SavedGame->GetSaveData();
	const TArray<FPlayerProgressData>& PlayerDataList = SavedData.PlayerProgressDataList;

	// 로드한거 적용 필요
	/*
	* 적용해야 할 것
	* PlayerNetId - UADGameInstance
	* Inventory - PlayerState
	* Upgrade - PlayerState
	* ClearCount - ADInGameState
	* TotalMoney - ADInGameState(TeamCredits)
	*/

	UWorld* World = GetWorld();
	check(World);

	AADInGameState* GS = Cast<AADInGameState>(World->GetGameState());
	if (GS == nullptr)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Load Failed, GameState is not valid, Slot : %s, UserIndex : %d"), *LoadedSlotName, UserIndex));
		return;
	}

	GS->SetClearCount(SavedData.ClearCount);
	GS->SetTotalTeamCredit(SavedData.TotalMoney);

	UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance());
	if (GI == nullptr)
	{
		PrintLogWithScreen(FString::Printf(TEXT("Load Failed, Gameinstance is not valid, Slot : %s, UserIndex : %d"), *LoadedSlotName, UserIndex));
		return;
	}

	for (const FPlayerProgressData& PlayerProgressData : SavedData.PlayerProgressDataList)
	{
		int32 PlayerIndex = INDEX_NONE;
		if (GI->TryGetPlayerIndex(PlayerProgressData.PlayerNetId, PlayerIndex) == false)
		{
			continue;
		}

		AADPlayerState* PS = Cast<AADPlayerState>(UGameplayStatics::GetPlayerState(World, PlayerIndex));
		if (PS == nullptr)
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Get PlayerState From Player Index %d"), PlayerIndex));
			continue;
		}

		UADInventoryComponent* InventoryComponent = PS->GetInventory();
		if (IsValid(InventoryComponent) == false || InventoryComponent->IsBeingDestroyed())
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Get InventoryComponent From Player Index %d"), PlayerIndex));
			continue;
		}

		UUpgradeComponent* UpgradeComponent = PS->GetUpgradeComp();
		if (IsValid(UpgradeComponent) == false || UpgradeComponent->IsBeingDestroyed())
		{
			PrintLogWithScreen(FString::Printf(TEXT("Fail to Get UpgradeComponent From Player Index %d"), PlayerIndex));
			continue;
		}

		for (const FItemData& ItemData : PlayerProgressData.ItemDataList)
		{
			InventoryComponent->AddInventoryItem(ItemData);
		}

		const int32 UpgradeTypeCount = PlayerProgressData.UpgradeGradeMap.Num();
		for (int32 i = 0; i < UpgradeTypeCount; ++i)
		{
			EUpgradeType UpgradeType = EUpgradeType(i);

			int32 Grade = PlayerProgressData.UpgradeGradeMap[i];
			if (UpgradeComponent->TrySetCurrentGrade(UpgradeType, Grade) == false)
			{
				PrintLogWithScreen(FString::Printf(TEXT("Fail to Set Upgrade, UpgradeType : %d, Grade : %d"), UpgradeType, Grade));
			}
		}
	}

	RestartWorld();
}

void USaveDataSubsystem::OnLoadedSavedGameListInfo(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame)
{
	bIsLoadingNow = false;

	if (IsServer() == false)
	{
		return;
	}

	FString ResultString = "";

	// 호스트가 아니면 불러오기 불가
	if (UserIndex != 0)
	{
		ResultString = FString::Printf(TEXT("Load Failed With Weird UserIndex : %d"), UserIndex);
		PrintLogWithScreen(ResultString);

		return;
	}

	USavedSessionInfoSaveGame* SavedSessionInfo = Cast<USavedSessionInfoSaveGame>(LoadedSavedGame);
	if (SavedSessionInfo == nullptr)
	{
		ResultString = FString::Printf(TEXT("Load Failed, Make Initial GameListInfo.. Slot : %s, UserIndex : %d"), *LoadedSlotName, UserIndex);
		PrintLogWithScreen(ResultString);

		CachedSavedSessionInfo = Cast<USavedSessionInfoSaveGame>(UGameplayStatics::CreateSaveGameObject(USavedSessionInfoSaveGame::StaticClass()));
		if (CachedSavedSessionInfo == nullptr)
		{
			PrintLogWithScreen(TEXT("Fail to Init GameList"));
		}
		else
		{
			AsyncSaveSavedGameListInfo();
		}

		return;
	}

	CachedSavedSessionInfo = SavedSessionInfo;
	ResultString = TEXT("Session Info Loaded");
	PrintLogWithScreen(ResultString);
	
	int64 SavedTime = 0;
	FString SavedSlotName = "";
	for (const auto& SavedInfo : CachedSavedSessionInfo->SavedSessionInfos)
	{
		if (SavedInfo.Value.SavedUnixTime > SavedTime)
		{
			SavedTime = SavedInfo.Value.SavedUnixTime;
			SavedSlotName = SavedInfo.Key;
		}
	}

	LastSavedGameSlotName = SavedSlotName;
}

void USaveDataSubsystem::PrintLogWithScreen(const FString& LogMessage)
{
	LOGV(Error, TEXT("%s"), *LogMessage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, *LogMessage);
	}
}

bool USaveDataSubsystem::IsServer() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return false;
	}

	ENetMode CurrentNetMode = World->GetNetMode();
	switch (CurrentNetMode)
	{
	case NM_Standalone:
		return true;
	case NM_DedicatedServer:
		return true;
	case NM_ListenServer:
		return true;
	case NM_Client:
		return false;
	case NM_MAX:
		check(false)
		return false;
	default:
		check(false);
		return false;
	}
}

void USaveDataSubsystem::RestartWorld()
{
	bIsLoadingNow = true;

	if (IsServer() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	UMissionSubsystem* MissionSubsystem = GetGameInstance()->GetSubsystem<UMissionSubsystem>();
	if (MissionSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to get MissionSubsystem"));
		return;
	}

	MissionSubsystem->RemoveAllMissions();

	AADInGameState* ADInGameState = UGameplayStatics::GetGameMode(World)->GetGameState<AADInGameState>();
	FString CampMapName = UGameplayStatics::GetCurrentLevelName(World, true);

	ADInGameState->SendDataToGameInstance();
	World->ServerTravel(CampMapName + TEXT("?listen"));
}

void USaveDataSubsystem::OnWorldTearDown(UWorld* World)
{
	bIsLoadingNow = false;
}

bool USaveDataSubsystem::IsExistSavedSessionGame(const FString& SlotName)
{
	return CachedSavedSessionInfo->SavedSessionInfos.Contains(SlotName);
}

USavedSessionInfoSaveGame* USaveDataSubsystem::GetSavedSesssionListInfo() const
{
	return CachedSavedSessionInfo;
}

bool USaveDataSubsystem::IsLoadingNow() const
{
	return bIsLoadingNow;
}

bool USaveDataSubsystem::IsSavingNow() const
{
	return bIsSavingNow;
}
