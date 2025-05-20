#include "UI/HoldInteractionWidget.h"
#include "Components/ProgressBar.h"

void UHoldInteractionWidget::HandleHoldStart(AActor* Target, float Duration)
{
    Elapsed = 0.f;
    bIsHolding = true;
    TotalDuration = Duration;
    if (HoldProgressBar)
    { 
        HoldProgressBar->SetPercent(0.f);
        AddToViewport();
    }
}

void UHoldInteractionWidget::HandleHoldCancel()
{
    bIsHolding = false;
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    RemoveFromParent();
}

void UHoldInteractionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsHolding || !HoldProgressBar)
    {
        return;
    }

    Elapsed += InDeltaTime;

    float Percent = FMath::Clamp(Elapsed / TotalDuration, 0.f, 1.f);

    HoldProgressBar->SetPercent(Percent);

    if (Percent >= 1.f)
    {
        HandleHoldCancel();
    }
}

