#include "SaveData/CoopData/CoopSessionSaveGame.h"

const FCoopSessionSaveData& UCoopSessionSaveGame::GetSaveData() const
{
	return SessionSaveData;
}

void UCoopSessionSaveGame::SetSaveData(const FCoopSessionSaveData& InSessionSaveData)
{
	SessionSaveData = InSessionSaveData;
}
