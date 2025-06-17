#include "UI/PlayerStatusWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Overlay.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"

const FName UPlayerStatusWidget::OnNextPhaseAnimFinishedName = TEXT("OnNextPhaseAnimFinished");
const int32 UPlayerStatusWidget::MaxPhaseNumber = 3;

UPlayerStatusWidget::UPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UPlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetSpearVisibility(false); 

    if (IsValid(NextPhaseAnim) == false)
    {
        LOGV(Error, TEXT("IsValid(NextPhaseAnim) == false"));
        return;
    }

    FWidgetAnimationDynamicEvent OnNextPhaseAnimFinishedDelegate;
    OnNextPhaseAnimFinishedDelegate.BindUFunction(this, OnNextPhaseAnimFinishedName);
    UnbindAllFromAnimationFinished(NextPhaseAnim);
    BindToAnimationFinished(NextPhaseAnim, OnNextPhaseAnimFinishedDelegate);
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

int8 UPlayerStatusWidget::GetNextPhaseAnimEndTime() const
{
    return NextPhaseAnim->GetEndTime();
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
        LOGV(Error, TEXT("OxygenBar is nullptr!"));
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
        LOGV(Error, TEXT("HealthBar is nullptr!"));
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
        LOGV(Error, TEXT("StaminaBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetDroneCurrentText(int32 Current)
{
    // 현재 가치 텍스트 갱신
    if (CurrentMoneyText && CurrentMoneyText->IsValidLowLevel())
    {
        CurrentMoneyText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Current)));
    }
}

void UPlayerStatusWidget::SetDroneTargetText(int32 Target)
{
    // 목표 가치 텍스트 갱신
    if (TargetMoneyText && TargetMoneyText->IsValidLowLevel())
    {
        TargetMoneyText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Target)));
    }
}

void UPlayerStatusWidget::PlayNextPhaseAnim(int32 NextPhaseNumber)
{
    // 나중에 테이블로 텍스트 정리할수도?
    if (NextPhaseNumber > MaxPhaseNumber)
    {
        SetNextPhaseText(TEXT("잠수정으로 돌아가라."));
    }
    else
    {
        SetNextPhaseText(FString::Printf(TEXT("Phase%d"), NextPhaseNumber));
    }

    CachedNextPhaseNumber = NextPhaseNumber;

    if (TryPlayAnim(NextPhaseAnim) == false)
    {
        return;
    }

    NextPhaseOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPlayerStatusWidget::SetCurrentPhaseText(const FString& PhaseText)
{
    CurrentPhaseText->SetText(FText::FromString(PhaseText));
}

void UPlayerStatusWidget::SetNextPhaseText(const FString& PhaseText)
{
    NextPhaseText->SetText(FText::FromString(PhaseText));
}

void UPlayerStatusWidget::SetCurrentPhaseOverlayVisible(bool bShouldVisible)
{
    if (bShouldVisible)
    {
        CurrentPhaseOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        CurrentPhaseOverlay->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerStatusWidget::OnNextPhaseAnimFinished()
{
    LOGV(Warning, TEXT("NextPhaseAnim Finished"));

    // 나중에 테이블로 텍스트 정리할수도?
    if (CachedNextPhaseNumber > MaxPhaseNumber)
    {
        SetCurrentPhaseText(TEXT("잠수정으로 돌아가라."));
    }
    else
    {
        SetCurrentPhaseText(FString::Printf(TEXT("Phase%d"), CachedNextPhaseNumber));
    }
}

bool UPlayerStatusWidget::TryPlayAnim(UWidgetAnimation* Anim)
{
    if (Anim == nullptr || IsValid(Anim) == false || Anim->IsValidLowLevel() == false)
    {
        return false;
    }

    PlayAnimation(Anim);

    return true;
}

void UPlayerStatusWidget::SetSpearVisibility(bool bVisible)
{
    if (SpearPanel)
    {
        SpearPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UPlayerStatusWidget::SetCompassObject(AActor* NewTargetObject)
{
    CompassTargetObject = NewTargetObject;
}

