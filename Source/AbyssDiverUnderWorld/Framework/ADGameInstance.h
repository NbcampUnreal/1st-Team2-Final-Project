#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ADGameInstance.generated.h"

enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UADGameInstance();

protected:

	virtual void Init() override;

public:

	bool TryGetPlayerIndex(const FString& NetId, int32& OutPlayerIndex);
	void AddPlayerNetId(const FString& NetId);
	void RemovePlayerNetId(const FString& NetId);

public:
	UPROPERTY(BlueprintReadWrite)
	EMapName SelectedLevelName;

	UPROPERTY(BlueprintReadWrite)
	int32 TeamCredits;


	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.DropEntry"))
	TObjectPtr<UDataTable> OreDropTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.UpgradeDataRow"))
	TObjectPtr<UDataTable> UpgradeDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.PhaseGoalRow"))
	TObjectPtr<UDataTable> PhaseGoalTable;


private:

	TMap<FString, int32> PlayerIdMap;
	TArray<bool> ValidPlayerIndexArray;

	const int32 MAX_PLAYER_NUMBER = 4;

};


