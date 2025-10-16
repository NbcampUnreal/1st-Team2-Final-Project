#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "UI/SelectedMissionListWidget.h"
#include "MissionSelectWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStartButtonClickedDelegate, const TArray<FMissionData>&/*SelectedMissions*/);
DECLARE_DELEGATE(FOnMisionResetButtonClickedDelegate);

class UButton;
class UScrollBox;
class UMissionEntryWidget;
class UBorder;
class UOverlay;

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
    void OnMissionResetButtonClicked();

    UFUNCTION()
    void OnMissionClicked(const FMissionData& Data, bool bSelected);


    // 내부 함수
    void AddMissionEntry(const FMissionData& Data);

    void UpdateEntrys();

    void UpdateMissionList(int8 CurrentLevelIndex, bool bIsUnlock);

	void UpdateSelectedMissionBox();

    FOnStartButtonClickedDelegate OnStartButtonClickedDelegate;
    FOnMisionResetButtonClickedDelegate OnMisionResetButtonClickedDelegate;
#pragma endregion

#pragma region Variable
protected:
    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ScrollBox_MissionList;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Start;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_MissionReset;

    // 미션 엔트리 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UMissionEntryWidget> MissionEntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<USelectedMissionListWidget> WBP_SelectedMissionListClass;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> WarningBorder;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> WarningText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> SelectedMissionsBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> LockImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> TouchDisplay;

    UPROPERTY(meta = (BindWidget))
    uint8 bIsMissionGained : 1;

private:
    // 미션 데이터
    TArray<FMissionData> AllMissions;
    TArray<FMissionData> SelectedMissions;
#pragma endregion
};
