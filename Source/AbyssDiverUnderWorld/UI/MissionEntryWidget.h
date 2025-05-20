#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "MissionEntryWidget.generated.h"

class UTextBlock;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionClicked, const FMissionData&, Mission, bool, bSelected);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 생성자 이후 호출 함수
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

#pragma region Method
public:
    void Setup(const FMissionData& InData);
    void SetSelected(bool bSelected);
    const FMissionData& GetMissionData() const { return MissionData; }

protected:
    virtual void NativeConstruct() override;
    void UpdateVisualState();
    void ApplyMissionData();
#pragma endregion

#pragma region Delegate
public:
    // 미션 선택 시 브로드캐스트
    UPROPERTY(BlueprintAssignable)
    FOnMissionClicked OnMissionClicked;
#pragma endregion

#pragma region Variable
protected:
    // 미션 제목 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_MissionTitle;

    // 잠금 힌트 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_UnlockHint;

    // 배경 UI
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_Background;

    // 테두리 UI
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_Outline;

private:
    // 선택 여부
    bool bIsSelected = false;

    // 미션 데이터
    FMissionData MissionData;
#pragma endregion
};
