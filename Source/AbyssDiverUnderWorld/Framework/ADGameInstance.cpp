#include "Framework/ADGameInstance.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/SoundSubsystem.h"
#include "DataRow/MonsterDexRow.h"

#include "Kismet/GameplayStatics.h"

//Data Rows
#include "DataRow/MissionDataRow/MissionBaseRow.h"
#include "DataRow/MissionDataRow/AggroTriggerMissionRow.h"
#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"
#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"
#include "DataRow/MissionDataRow/ItemUseMissionRow.h"
#include "DataRow/MissionDataRow/InteractionMissionRow.h"

const int32 UADGameInstance::MAX_PLAYER_NUMBER = 4;

template<typename TRow>
static void GetRowsByLevel(const UDataTable* Table, ELevelName  LevelName, TArray<TRow>& Out)
{
    Out.Reset();
    if (!Table)
        return;

    TArray<TRow*> AllRows;
    Table->GetAllRows<TRow>(TEXT("MissionQuery"), AllRows);
    for (const TRow* R : AllRows)
    {
        if (!R)
            continue;
		// None이면 모든 레벨에서 허용
        if (R->LevelName == ELevelName::None || R->LevelName == LevelName)
        {
            Out.Add(*R);
        }
    }
}




UADGameInstance::UADGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UADGameInstance::Init()
{
    Super::Init();

	bIsHost = false;

    InitPlayerInfos();

    SettingsManager = NewObject<USettingsManager>(this);
    SettingsManager->LoadAllSettings(GetFirstLocalPlayerController());

    SettingsManager->InitializeActionMap(GetInputActionMap());

    if (UGameplayStatics::DoesSaveGameExist(UADTutorialSaveGame::SaveSlotName, UADTutorialSaveGame::UserIndex))
    {
        TutorialSaveGameObject = Cast<UADTutorialSaveGame>(UGameplayStatics::LoadGameFromSlot(UADTutorialSaveGame::SaveSlotName, UADTutorialSaveGame::UserIndex));
    }
    else
    {
        TutorialSaveGameObject = Cast<UADTutorialSaveGame>(UGameplayStatics::CreateSaveGameObject(UADTutorialSaveGame::StaticClass()));
        UGameplayStatics::SaveGameToSlot(TutorialSaveGameObject, UADTutorialSaveGame::SaveSlotName, UADTutorialSaveGame::UserIndex);
    }
}


void UADGameInstance::InitPlayerInfos()
{
    CurrentPlayerIdMap.Empty(MAX_PLAYER_NUMBER);
    ValidPlayerIndexArray.Init(false, MAX_PLAYER_NUMBER);

    TotalPlayerIdSet.Empty(MAX_PLAYER_NUMBER);
}

bool UADGameInstance::TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex)
{
    if (CurrentPlayerIdMap.Contains(NetId))
    {
        OutPlayerIndex = CurrentPlayerIdMap[NetId];
        return true;
    }

    return false;
}

void UADGameInstance::AddPlayerNetId(const FString& NetId)
{
    for (int32 i = 0; i < MAX_PLAYER_NUMBER; ++i)
    {
        if (ValidPlayerIndexArray[i])
        {
            continue;
        }

        ValidPlayerIndexArray[i] = true;
        CurrentPlayerIdMap.Add(NetId, i);
        TotalPlayerIdSet.Add(NetId);

        return;
    }

    LOGV(Warning, TEXT("Already Exist Id : %s"), *NetId);
}

void UADGameInstance::RemovePlayerNetId(const FString& NetId)
{
    int32 PlayerIndex = 0;
    if(TryGetPlayerIndex(NetId, PlayerIndex) == false)
    {
        LOGV(Warning, TEXT("Remove Failed Because of Not Valid NetId"));
        return;
    }

    ValidPlayerIndexArray[PlayerIndex] = false;
    CurrentPlayerIdMap.Remove(NetId);
}

bool UADGameInstance::HasBeenVisited(const FString& NetId)
{
    return TotalPlayerIdSet.Contains(NetId);
}

void UADGameInstance::ChangeMasterVolume(const float& NewVolume)
{
    GetSubsystem<USoundSubsystem>()->ChangeMasterVolume(NewVolume);
}

void UADGameInstance::ChangeBGMVolume(const float& NewVolume)
{
    GetSubsystem<USoundSubsystem>()->ChangeBGMVolume(NewVolume);
}

void UADGameInstance::ChangeSFXVolume(const float& NewVolume)
{
    GetSubsystem<USoundSubsystem>()->ChangeSFXVolume(NewVolume);
}

void UADGameInstance::ChangeAmbientVolume(const float& NewVolume)
{
    GetSubsystem<USoundSubsystem>()->ChangeAmbientVolume(NewVolume);
}

void UADGameInstance::GetMissionsForLevel(
    ELevelName LevelName,
    TArray<FInteractionMissionRow>& OutInteractions,
    TArray<FItemUseMissionRow>& OutItemUses,
    TArray<FItemCollectMissionRow>& OutCollections,
    TArray<FKillMonsterMissionRow>& OutKills,
    TArray<FAggroTriggerMissionRow>& OutAggros) const
{
    GetRowsByLevel(InteractionMissionTable, LevelName, OutInteractions);
    GetRowsByLevel(ItemUseMissionTable, LevelName, OutItemUses);
    GetRowsByLevel(ItemCollectMissionTable, LevelName, OutCollections);
    GetRowsByLevel(KillMonsterMissionTable, LevelName, OutKills);
    GetRowsByLevel(AggroTriggerMissionTable, LevelName, OutAggros);
}

template<typename T>
static void PickRandomInPlace(TArray<T>& InOut, int32 MaxCount)
{
    if (MaxCount <= 0 || InOut.Num() <= MaxCount) return;
    InOut.Sort([](const T&, const T&) { return FMath::RandBool(); }); // 셔플
    InOut.SetNum(MaxCount, /*bAllowShrinking=*/false);
}

void UADGameInstance::GetRandomMissionsForLevel(
    ELevelName LevelName, int32 MaxCountPerType,
    TArray<FInteractionMissionRow>& OutInteractions,
    TArray<FItemUseMissionRow>& OutItemUses,
    TArray<FItemCollectMissionRow>& OutCollections,
    TArray<FKillMonsterMissionRow>& OutKills,
    TArray<FAggroTriggerMissionRow>& OutAggros) const
{
    GetMissionsForLevel(LevelName, OutInteractions, OutItemUses, OutCollections, OutKills, OutAggros);
    PickRandomInPlace(OutInteractions, MaxCountPerType);
    PickRandomInPlace(OutItemUses, MaxCountPerType);
    PickRandomInPlace(OutCollections, MaxCountPerType);
    PickRandomInPlace(OutKills, MaxCountPerType);
    PickRandomInPlace(OutAggros, MaxCountPerType);
}


const float UADGameInstance::GetCurrentMasterVolume() const
{
    return GetSubsystem<USoundSubsystem>()->GetMasterVolume();
}

const float UADGameInstance::GetCurrentBGMVolume() const
{
    return GetSubsystem<USoundSubsystem>()->GetBGMVolume();
}

const float UADGameInstance::GetCurrentSFXVolume() const
{
    return GetSubsystem<USoundSubsystem>()->GetSFXVolume();
}

const float UADGameInstance::GetCurrentAmbientVolume() const
{
    return GetSubsystem<USoundSubsystem>()->GetAmbientVolume();
}

const TMap<FString, int32>& UADGameInstance::GetCurrentPlayerIdMap() const
{
    return CurrentPlayerIdMap;
}