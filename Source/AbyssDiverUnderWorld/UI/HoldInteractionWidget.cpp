#include "UI/HoldInteractionWidget.h"
#include "Components/ProgressBar.h"

void UHoldInteractionWidget::HandleHoldStart(AActor* Target, float Duration)
{
    Elapsed = 0.f;
    if (HoldProgressBar)
    {
        HoldProgressBar->SetPercent(0.f);
        // �� 0.01�ʸ��� UpdateProgress ȣ��
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &UHoldInteractionWidget::UpdateProgress, Duration),
            0.01f, true);

        SetVisibility(ESlateVisibility::Visible);
    }
}

void UHoldInteractionWidget::HandleHoldCancel(AActor* Target)
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
        // �Ϸ� �� �ڵ� ���
        HandleHoldCancel(nullptr);
    }
}
