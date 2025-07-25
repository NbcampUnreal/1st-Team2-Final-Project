#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SaveDataSubsystem.generated.h"

class UCoopSessionSaveGame;
class USaveGame;
class USavedSessionInfoSaveGame;

struct FSavedSessionInfo;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USaveDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Methods

public:

	// Auto Save 전용
	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void AsyncSaveCurrentGame();
	void AsyncSaveCurrentGame(const FString& SaveGameName, bool bShouldOverwrite);

	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void AsyncLoadLastSavedGame();
	void AsyncLoadSavedGame(const FString& SavedSlotName);
	
	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void AsyncSaveSavedGameListInfo();

	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void AsyncLoadSavedGameListInfo();

	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void DeleteAllSavedGame();
	void DeleteSavedGame(const FString& SavedSlotName);

	UFUNCTION(BlueprintCallable, Category = "SaveDataSubsystem")
	void DisplayAllSaves();

private:

	void OnSaveSessionGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded);
	void OnSaveSavedGameListInfoGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded);

	// Load에 실패시 LoadedSavedGame는 nullptr로 들어온다.
	void OnLoadedSavedGame(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame);

	// Load에 실패시 LoadedSavedGame는 nullptr로 들어온다.
	void OnLoadedSavedGameListInfo(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame);

	void PrintLogWithScreen(const FString& LogMessage);

	bool IsServer() const;

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY()
	TObjectPtr<USavedSessionInfoSaveGame> CachedSavedSessionInfo;

	UPROPERTY()
	TObjectPtr<UCoopSessionSaveGame> LastSavedSessionGame;

	FString LastSavedGameSlotName;


	static const FString SavedSessionInfoSlotName;
	static const FString SaveSlotNamePrefix;

#pragma endregion

#pragma region Getter / Setter

public:

	bool IsExistSavedSessionGame(const FString& SlotName);

	USavedSessionInfoSaveGame* GetSavedSesssionListInfo() const;

#pragma endregion

	
};
