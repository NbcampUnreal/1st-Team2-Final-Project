#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "SavedSessionInfoSaveGame.generated.h"

USTRUCT()
struct FSavedSessionInfo
{
	GENERATED_BODY()

public:

	FSavedSessionInfo()
	{

	}

	FSavedSessionInfo(const FString& InSaveName)
	{
		SaveName = InSaveName;

		const FDateTime Now = FDateTime::UtcNow();
		SavedUnixTime = Now.ToUnixTimestamp();

	}

	UPROPERTY()
	FString SaveName;

	UPROPERTY()
	int64 SavedUnixTime;

	bool operator<(const FSavedSessionInfo& Other)
	{
		return SavedUnixTime < Other.SavedUnixTime;
	}
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USavedSessionInfoSaveGame : public USaveGame
{
	GENERATED_BODY()

#pragma region Methods

public:

	void AddSessionInfo(const FString& SavedSlotName, const FSavedSessionInfo& SessionInfo);
	void RemoveSessionInfo(const FString& SavedSlotName);


#pragma endregion

#pragma region Variables

public:

	// SavedSlotName, SessionInfo
	UPROPERTY()
	TMap<FString, FSavedSessionInfo> SavedSessionInfos;

#pragma endregion
};
