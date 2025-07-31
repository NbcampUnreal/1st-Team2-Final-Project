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

	FSavedSessionInfo(const FString& InSaveName, int32 InClearCount, const TArray<FString>& InSavedPlayerNameTextList)
	{
		SaveName = InSaveName;
		ClearCount = InClearCount;
		SavedPlayerNameTextList = InSavedPlayerNameTextList;

		const FDateTime Now = FDateTime::UtcNow();
		SavedUnixTime = Now.ToUnixTimestamp();

	}

public:

	UPROPERTY()
	FString SaveName;

	UPROPERTY()
	int64 SavedUnixTime;

	UPROPERTY()
	int32 ClearCount;

	UPROPERTY()
	TArray<FString> SavedPlayerNameTextList;

	bool operator<(const FSavedSessionInfo& Other)
	{
		return SavedUnixTime < Other.SavedUnixTime;
	}

	FString GetTimestempString() const
	{
		FDateTime DateTime = FDateTime::FromUnixTimestamp(SavedUnixTime);
		return DateTime.ToString(TEXT("[%Y-%m-%d %H:%M:%S]"));
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
