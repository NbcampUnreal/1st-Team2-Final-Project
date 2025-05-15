#include "UI/SpearCountHudWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

USpearCountHudWidget::USpearCountHudWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USpearCountHudWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetSpearCount(CurrentSpear, TotalSpearCount);
    SetOxygenPercent(1.0f); // 시작 시 100%
}

void USpearCountHudWidget::SetSpearCount(int32 Current, int32 Total)
{
    CurrentSpear = Current;
    TotalSpearCount = Total;

    if (SpearNum)
    {
        SpearNum->SetText(FText::AsNumber(CurrentSpear));
    }

    if (TotalSpear)
    {
        TotalSpear->SetText(FText::AsNumber(TotalSpearCount));
    }
}

void USpearCountHudWidget::SetCurrentSpear(int32 InValue)
{
    SetSpearCount(InValue, TotalSpearCount);
}

void USpearCountHudWidget::SetTotalSpear(int32 InValue)
{
    SetSpearCount(CurrentSpear, InValue);
}

void USpearCountHudWidget::SetOxygenPercent(float InPercent)
{
    if (OxygenBar)
    {
        OxygenBar->SetPercent(FMath::Clamp(InPercent, 0.0f, 1.0f));
    }
}
