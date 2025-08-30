#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SaveDataSubsystem.generated.h"

class UCoopSessionSaveGame;
class USaveGame;
class USavedSessionInfoSaveGame;

struct FSavedSessionInfo;

DECLARE_MULTICAST_DELEGATE(FOnSaveCompletedDelegate);
DECLARE_MULTICAST_DELEGATE(FOnLoadCompletedDelegate);

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

	// Quick Save 전용
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

	bool SaveDexBits(const TArray<uint8>& InBits);

	bool LoadDexBits(TArray<uint8>& OutBits);


	FOnSaveCompletedDelegate OnSaveCompletedDelegate;
	FOnLoadCompletedDelegate OnLoadCompletedDelegate;

private:

	void OnSaveSessionGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded);
	void OnSaveSavedGameListInfoGame(const FString& SlotName, const int32 UserIndex, bool bIsSucceeded);

	// Load에 실패시 LoadedSavedGame는 nullptr로 들어온다.
	void OnLoadedSavedGame(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame);

	// Load에 실패시 LoadedSavedGame는 nullptr로 들어온다.
	void OnLoadedSavedGameListInfo(const FString& LoadedSlotName, const int32 UserIndex, USaveGame* LoadedSavedGame);

	void PrintLogWithScreen(const FString& LogMessage);

	bool IsServer() const;

	void RestartWorld();

	void OnWorldTearDown(UWorld* World);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY()
	TObjectPtr<USavedSessionInfoSaveGame> CachedSavedSessionInfo;

	UPROPERTY()
	TObjectPtr<UCoopSessionSaveGame> LastSavedSessionGame;

	FString LastSavedGameSlotName;

	uint8 bIsLoadingNow : 1 = false;
	uint8 bIsSavingNow : 1 = false;

	static const FString SavedSessionInfoSlotName;
	static const FString SaveSlotNamePrefix;

	FString DexSaveSlot = TEXT("DexBitsSave");

#pragma endregion

#pragma region Getter / Setter

public:

	bool IsExistSavedSessionGame(const FString& SlotName);

	// 존재하면 해당하는 SlotName 반환, 없으면 빈 문자열 "" 반환
	FString GetSavedSlotNameFromSaveName(const FString& SaveName);

	USavedSessionInfoSaveGame* GetSavedSesssionListInfo() const;

	bool IsLoadingNow() const;
	bool IsSavingNow() const;

#pragma endregion

	
};
