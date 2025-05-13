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

	UPROPERTY(EditDefaultsOnly, Category = "ADTestGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.DropEntry"))
	TObjectPtr<UDataTable> OreDropTable;

	UPROPERTY(EditDefaultsOnly, Category = "ADTestGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.UpgradeDataRow"))
	TObjectPtr<UDataTable> UpgradeDataTable;

#pragma endregion

};
