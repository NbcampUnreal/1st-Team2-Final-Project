#include "Framework/ADGameInstance.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/SoundSubsystem.h"

#include "Kismet/GameplayStatics.h"

const int32 UADGameInstance::MAX_PLAYER_NUMBER = 4;

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
    SettingsManager->LoadAllSettings(true);
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