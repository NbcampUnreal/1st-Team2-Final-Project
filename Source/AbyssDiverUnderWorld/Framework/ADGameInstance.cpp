#include "Framework/ADGameInstance.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/SoundSubsystem.h"

#include "Kismet/GameplayStatics.h"

UADGameInstance::UADGameInstance()
{
}

void UADGameInstance::Init()
{
    Super::Init();

	bIsHost = false;
    PlayerIdMap.Empty(MAX_PLAYER_NUMBER);
    ValidPlayerIndexArray.Init(false, MAX_PLAYER_NUMBER);

    SettingsManager = NewObject<USettingsManager>(this);
    SettingsManager->LoadAllSettings(GetFirstLocalPlayerController());

    SettingsManager->InitializeActionMap(GetInputActionMap());
}

bool UADGameInstance::TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex)
{
    if (PlayerIdMap.Contains(NetId))
    {
        OutPlayerIndex = PlayerIdMap[NetId];
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
        PlayerIdMap.Add(NetId, i + 1);
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

    ValidPlayerIndexArray[PlayerIndex - 1] = false;
    PlayerIdMap.Remove(NetId);
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
