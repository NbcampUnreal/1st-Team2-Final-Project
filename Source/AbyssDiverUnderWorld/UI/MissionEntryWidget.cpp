#include "UI/MissionEntryWidget.h"
#include "Components/TextBlock.h"
#include "UI/MissionData.h"
#include "Components/Border.h"

void UMissionEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();


}

void UMissionEntryWidget::Setup(const FMissionData& InData)
{
    MissionData = InData;
    bIsSelected = false;

    if (Text_MissionTitle)
    {
        Text_MissionTitle->SetText(FText::FromString(MissionData.Title));
        Text_MissionTitle->SetVisibility(ESlateVisibility::Visible);
    }

    ApplyMissionData();
    UpdateVisualState();
}

void UMissionEntryWidget::ApplyMissionData()
{
    if (!Text_MissionTitle || !Text_UnlockHint || !Border_Background || !Border_Outline)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [MissionEntryWidget] 필수 위젯 바인딩 실패"));
        return;
    }

    if (!MissionData.bIsUnlocked)
    {
        Text_MissionTitle->SetText(FText::FromString(TEXT("???")));
        Text_UnlockHint->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Warning, TEXT("🔒 [MissionEntryWidget] 잠긴 미션 표시: ???"));
    }
    else
    {
        Text_MissionTitle->SetText(FText::FromString(MissionData.Title));
        UE_LOG(LogTemp, Warning, TEXT("✅ [MissionEntryWidget] 미션 제목 표시: %s"), *MissionData.Title);

        Text_UnlockHint->SetText(FText::FromString(MissionData.UnlockHint));
        Text_UnlockHint->SetVisibility(ESlateVisibility::Visible);
    }
}



FReply UMissionEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (!MissionData.bIsUnlocked)
        {
            return FReply::Handled();
        }

        bIsSelected = !bIsSelected;
        UpdateVisualState();
        OnMissionClicked.Broadcast(MissionData, bIsSelected);

        TSharedPtr<SWidget> Cached = GetCachedWidget();
        if (Cached.IsValid())
        {
            return FReply::Handled().SetUserFocus(Cached.ToSharedRef(), EFocusCause::SetDirectly);
        }

        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMissionEntryWidget::UpdateVisualState()
{
    if (!Text_MissionTitle || !Border_Background || !Border_Outline)
        return;

    if (bIsSelected)
    {
		if (SelectedImage)
		{
			Border_Background->SetBrushFromTexture(SelectedImage);
		}
    }
    else
    {
		if (UnselectedImage)
		{
			Border_Background->SetBrushFromTexture(UnselectedImage);
		}
    }
}

void UMissionEntryWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisualState();
}
