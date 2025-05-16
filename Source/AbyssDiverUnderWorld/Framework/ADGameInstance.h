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

};


