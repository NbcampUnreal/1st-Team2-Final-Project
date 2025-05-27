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

        FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold", 28);
        Text_MissionTitle->SetFont(FontInfo);
        Text_MissionTitle->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
    }

    SetRenderOpacity(1.f);
    SetRenderScale(FVector2D(1.f, 1.f));
    SetVisibility(ESlateVisibility::Visible);

    if (Border_Background)
        Border_Background->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 1.f)); // 불투명!

    ApplyMissionData();
    UpdateVisualState();
}




void UMissionEntryWidget::ApplyMissionData()
{
    if (!Text_MissionTitle)
        UE_LOG(LogTemp, Error, TEXT("❌ Text_MissionTitle 바인딩 안됨"));
    if (!Text_UnlockHint)
        UE_LOG(LogTemp, Error, TEXT("❌ Text_UnlockHint 바인딩 안됨"));
    if (!Border_Background)
        UE_LOG(LogTemp, Error, TEXT("❌ Border_Background 바인딩 안됨"));
    if (!Border_Outline)
        UE_LOG(LogTemp, Error, TEXT("❌ Border_Outline 바인딩 안됨"));

    if (!Text_MissionTitle || !Text_UnlockHint || !Border_Background || !Border_Outline)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [MissionEntryWidget] 필수 위젯 바인딩 실패"));
        return;
    }

    if (MissionData.Stage == 3 && !MissionData.bIsUnlocked)
    {
        Text_MissionTitle->SetText(FText::FromString(TEXT("???")));
        Text_UnlockHint->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Warning, TEXT("🔒 [MissionEntryWidget] 잠긴 미션 표시: ???"));
    }
    else
    {
        Text_MissionTitle->SetText(FText::FromString(MissionData.Title));
        UE_LOG(LogTemp, Warning, TEXT("✅ [MissionEntryWidget] 미션 제목 표시: %s"), *MissionData.Title);

        if (MissionData.Stage == 3)
        {
            Text_UnlockHint->SetText(FText::FromString(MissionData.UnlockHint));
            Text_UnlockHint->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            Text_UnlockHint->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // 기본 배경
    Border_Background->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

    FLinearColor OutlineColor =
        MissionData.Stage == 1 ? FLinearColor::Yellow :
        MissionData.Stage == 2 ? FLinearColor(1.f, 0.5f, 0.f) :
        FLinearColor::Red;

    Border_Outline->SetBrushColor(OutlineColor);
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

    FSlateFontInfo FontInfo = Text_MissionTitle->GetFont();
    FontInfo.TypefaceFontName = bIsSelected ? FName("Bold") : FName("Regular");
    FontInfo.Size = 24;
    Text_MissionTitle->SetFont(FontInfo);

    if (bIsSelected)
    {
        Border_Background->SetBrushColor(FLinearColor(0.0f, 0.25f, 0.0f, 1.0f));
        Border_Outline->SetBrushColor(FLinearColor(0.0f, 1.0f, 0.3f, 1.0f));
    }
    else
    {
        Border_Background->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

        FLinearColor OutlineColor;
        if (MissionData.Stage == 1)
            OutlineColor = FLinearColor::Yellow;
        else if (MissionData.Stage == 2)
            OutlineColor = FLinearColor(1.f, 0.5f, 0.f);
        else
            OutlineColor = FLinearColor::Red;

        Border_Outline->SetBrushColor(OutlineColor);
    }
}

void UMissionEntryWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisualState();
}
