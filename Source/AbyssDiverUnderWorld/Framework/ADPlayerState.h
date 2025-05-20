#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/MissionData.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    // 생성자
    AADPlayerState();

protected:
    // 생명주기 함수
    virtual void BeginPlay() override;
    virtual void PostNetInit() override;

#pragma region Method
public:
    // 미션 관련 함수
    const TArray<FMissionData>& GetSelectedMissions() const { return SelectedMissions; }
    void SetSelectedMissions(const TArray<FMissionData>& NewMissions) { SelectedMissions = NewMissions; }
#pragma endregion

#pragma region Variable
protected:
    // 인벤토리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UADInventoryComponent> InventoryComp;

    // 선택된 미션 목록
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
    TArray<FMissionData> SelectedMissions;
#pragma endregion

#pragma region Getter/Setter
public:
    UADInventoryComponent* GetInventory() { return InventoryComp; }
#pragma endregion
};
