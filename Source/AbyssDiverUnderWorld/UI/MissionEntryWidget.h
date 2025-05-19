#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "MissionEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionClicked, const FMissionData&, Mission, bool, bSelected);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void Setup(const FMissionData& InData);
    void SetSelected(bool bSelected);
    const FMissionData& GetMissionData() const { return MissionData; }

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    UPROPERTY(BlueprintAssignable)
    FOnMissionClicked OnMissionClicked;

protected:
    virtual void NativeConstruct() override;
    void UpdateVisualState();
    void ApplyMissionData(); 

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_MissionTitle;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_UnlockHint;

    UPROPERTY(meta = (BindWidget))
    class UBorder* Border_Background;

    UPROPERTY(meta = (BindWidget))
    class UBorder* Border_Outline;

private:
    bool bIsSelected = false;
    FMissionData MissionData;
};
