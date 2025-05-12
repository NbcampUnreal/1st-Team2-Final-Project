#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "ADTestGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UADTestGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

#pragma region Variables


public:

	UPROPERTY(EditDefaultsOnly, Category = "ADTestGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADTestGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FDropEntry"))
	TObjectPtr<UDataTable> OreDropTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADTestGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADUpgradeDataRow"))
	TObjectPtr<UDataTable> UpgradeDataTable;

#pragma endregion

};
