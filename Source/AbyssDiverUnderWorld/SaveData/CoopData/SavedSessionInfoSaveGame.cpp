#include "SaveData/CoopData/SavedSessionInfoSaveGame.h"

void USavedSessionInfoSaveGame::AddSessionInfo(const FString& SavedSlotName, const FSavedSessionInfo& SessionInfo)
{
	SavedSessionInfos.Add({ SavedSlotName, SessionInfo });
}

void USavedSessionInfoSaveGame::RemoveSessionInfo(const FString& SavedSlotName)
{
	SavedSessionInfos.Remove(SavedSlotName);
}
