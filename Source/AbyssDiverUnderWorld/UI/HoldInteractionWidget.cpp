#include "UI/HoldInteractionWidget.h"
#include "Components/ProgressBar.h"

void UHoldInteractionWidget::HandleHoldStart(AActor* Target, float Duration)
{
    Elapsed = 0.f;
    if (HoldProgressBar)
    {
        HoldProgressBar->SetPercent(0.f);
        // 매 0.01초마다 UpdateProgress 호출
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &UHoldInteractionWidget::UpdateProgress, Duration),
            0.01f, true);

        AddToViewport();
    }
}

void UHoldInteractionWidget::HandleHoldCancel()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    RemoveFromParent();
}

void UHoldInteractionWidget::UpdateProgress(float Duration)
{
    Elapsed += GetWorld()->GetDeltaSeconds();
    float Percent = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
    HoldProgressBar->SetPercent(Percent);

    if (Percent >= 1.f)
    {
        // 완료 시 자동 취소
        HandleHoldCancel();
    }
}
