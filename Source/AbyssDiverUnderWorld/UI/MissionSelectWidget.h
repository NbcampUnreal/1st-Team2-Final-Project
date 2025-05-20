#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "UI/SelectedMissionListWidget.h"
#include "MissionSelectWidget.generated.h"

class UButton;
class UScrollBox;
class UMissionEntryWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

#pragma region Method
public:
    // 콜백
    UFUNCTION()
    void OnStartButtonClicked();

    UFUNCTION()
    void OnMissionClicked(const FMissionData& Data, bool bSelected);

    // 내부 함수
    void AddMissionEntry(const FMissionData& Data);
#pragma endregion

#pragma region Variable
protected:
    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ScrollBox_MissionList;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Start;

    // 미션 엔트리 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UMissionEntryWidget> MissionEntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<USelectedMissionListWidget> WBP_SelectedMissionListClass;


private:
    // 미션 데이터
    TArray<FMissionData> AllMissions;
    TArray<FMissionData> SelectedMissions;
#pragma endregion
};
