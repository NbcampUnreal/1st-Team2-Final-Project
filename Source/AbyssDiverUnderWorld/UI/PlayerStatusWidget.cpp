#include "UI/PlayerStatusWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

UPlayerStatusWidget::UPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UPlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetSpearVisibility(false); 
}

void UPlayerStatusWidget::SetSpearCount(int32 Current, int32 Total)
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

void UPlayerStatusWidget::SetCurrentSpear(int32 InValue)
{
    SetSpearCount(InValue, TotalSpearCount);
}

void UPlayerStatusWidget::SetTotalSpear(int32 InValue)
{
    SetSpearCount(CurrentSpear, InValue);
}

void UPlayerStatusWidget::SetOxygenPercent(float InPercent)
{
    if (OxygenBar)
    {
        OxygenBar->SetPercent(FMath::Clamp(InPercent, 0.0f, 1.0f));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OxygenBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetHealthPercent(float InPercent)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(FMath::Clamp(InPercent, 0.0f, 1.0f));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetStaminaPercent(float InPercent)
{
    if (StaminaBar)
    {
        const float Clamped = FMath::Clamp(InPercent, 0.0f, 1.0f);
        StaminaBar->SetPercent(Clamped);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ StaminaBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetSpearVisibility(bool bVisible)
{
    if (SpearPanel)
    {
        SpearPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}
