#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MissionData.h"
#include "ADGameInstance.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    void SetSelectedMissions(const TArray<FMissionData>& Missions) { SelectedMissions = Missions; }
    const TArray<FMissionData>& GetSelectedMissions() const { return SelectedMissions; }

#pragma region Variable

public:
    // 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.FADItemDataRow"))
    TObjectPtr<UDataTable> ItemDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.DropEntry"))
    TObjectPtr<UDataTable> OreDropTable;

    UPROPERTY(EditDefaultsOnly, Category = "ADGameInstance", meta = (RequiredAssetDataTags = "RowStructure=/Script/AbyssDiverUnderWorld.UpgradeDataRow"))
    TObjectPtr<UDataTable> UpgradeDataTable;

    // 선택된 미션을 저장할 배열
    UPROPERTY(BlueprintReadWrite)
    TArray<FMissionData> SelectedMissions;

#pragma endregion
};
