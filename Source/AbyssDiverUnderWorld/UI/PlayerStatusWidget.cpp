#include "UI/PlayerStatusWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Overlay.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Projectile/ADSpearGunBullet.h"
#include "UI/WarningWidget.h"
#include "UI/NoticeWidget.h"
#include "UI/DepthWidget.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"

const FName UPlayerStatusWidget::OnNextPhaseAnimFinishedName = TEXT("OnNextPhaseAnimFinished");

UPlayerStatusWidget::UPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Game/_AbyssDiver/Materials/M_Hit"));
    if (MaterialFinder.Succeeded())
    {
        LoadedMaterial = MaterialFinder.Object;
    }
}

void UPlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetSpearVisibility(false); 

    SetTopNameEmpty(); // Map Transition 시 이전 플레이어 이름이 남아있는 문제 방지
	FTimerHandle DelayBindTimerHandle;
	float DelayTime = 1.0f; 
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayBindTimerHandle, [this]()
        {
            UWorld* World = GetWorld();
            if (!IsValid(this) || !IsValid(World))
            {
                return;
            }
        
            if (AADInGameState* GameState = Cast<AADInGameState>(GetWorld()->GetGameState()))
            {
                GameState->OnTopMinerChangedDelegate.AddUObject(this, &UPlayerStatusWidget::SetTopName);
            }
        }, DelayTime, false);

    const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
    // MainLevel (Camp) 일 때 Hide
    UE_LOG(AbyssDiver, Warning, TEXT("Current Map Name: %s"), *CurrentMapName);
    if (CurrentMapName == "Submarine_Lobby")
    {
        TopNameOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        TopNameOverlay->SetVisibility(ESlateVisibility::Visible);
    }
    
    if (IsValid(NextPhaseAnim))
    {
        FWidgetAnimationDynamicEvent OnNextPhaseAnimFinishedDelegate;
        OnNextPhaseAnimFinishedDelegate.BindUFunction(this, OnNextPhaseAnimFinishedName);
        UnbindAllFromAnimationFinished(NextPhaseAnim);
        BindToAnimationFinished(NextPhaseAnim, OnNextPhaseAnimFinishedDelegate);
    }
    else
    {
        LOGV(Error, TEXT("NextPhaseAnim is invalid"));
    }

    if (HealthScreenEffect && LoadedMaterial)
    {
        // 다이나믹 머티리얼 인스턴스 생성
        DynamicMaterial = UMaterialInstanceDynamic::Create(LoadedMaterial, this);

        // Brush에 머티리얼 적용
        FSlateBrush NewBrush;
        NewBrush.SetResourceObject(DynamicMaterial);
        HealthScreenEffect->SetBrush(NewBrush);
    }
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
        // 1. 게이지 스케일
        float ClampedPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
        FWidgetTransform Transform = OxygenBar->GetRenderTransform();
        Transform.Scale = FVector2D(1.0f, ClampedPercent);
        OxygenBar->SetRenderTransform(Transform);

        // 2. 상단만 코너 반경 적용
        float TopRadius = 0.0f;
        if (ClampedPercent > 0.9f && ClampedPercent < 1.0f)
        {
            // 0.9~1.0 사이일 때 0~15로 보간
            float Alpha = (ClampedPercent - 0.9f) / 0.1f; // 0~1 구간
            TopRadius = FMath::Lerp(0.0f, 15.0f, Alpha);
        }
        else if (ClampedPercent >= 1.0f)
        {
            TopRadius = 15.0f;
        }

        // 스타일 적용
        FProgressBarStyle Style = OxygenBar->GetWidgetStyle();

        // 1. 배경
        Style.BackgroundImage.OutlineSettings.CornerRadii = FVector4(TopRadius, TopRadius, 0.0f, 0.0f);
        Style.BackgroundImage.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;

        // 2. 채우기 영역
        Style.FillImage.OutlineSettings.CornerRadii = FVector4(TopRadius, TopRadius, 0.0f, 0.0f);
        Style.FillImage.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;


        float WarningOxygenValue = 0.5f;

        if (ClampedPercent <= WarningOxygenValue)
        {

            float GreenBlueValue = FMath::Clamp(ClampedPercent- 0.1f, 0.0f, WarningOxygenValue - 0.1f) / WarningOxygenValue - 0.1f;
            Style.FillImage.TintColor = FSlateColor(FLinearColor(1.0f, GreenBlueValue, GreenBlueValue, 1.0f));
        }
        else
        {
            Style.FillImage.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (ClampedPercent <= (WarningOxygenValue - 0.2f))
        {
            if (!OxygenWarningWidget->GetbShowWarning())
            {
                OxygenWarningWidget->SetbShowWarning(true);
            }
        }
        else
        {
            if (OxygenWarningWidget->GetbShowWarning())
            {
                OxygenWarningWidget->SetbShowWarning(false);
                //DynMat->SetScalarParameterValue(FName("Period"), 0);
            }
        }

        OxygenBar->SetWidgetStyle(Style);
    }
    else
    {
        LOGV(Error, TEXT("OxygenBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetHealthPercent(float InPercent)
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("Range", 1-FMath::Clamp(InPercent, 0.0f, 1.0f));
    }
    else
    {
        LOGV(Error, TEXT("HealthBar is nullptr!"));
    }
}

void UPlayerStatusWidget::SetStaminaPercent(float InPercent)
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

void UPlayerStatusWidget::SetDroneCurrentText(int32 Current)
{
    // 현재 가치 텍스트 갱신
    if (CurrentMoneyText && CurrentMoneyText->IsValidLowLevel())
    {
        CurrentMoneyText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Current)));
        PlayAnimation(IncreaseMoney);
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

void UPlayerStatusWidget::SetMoneyProgressBar(float InPercent)
{
    if (MoneyProgressBar)
    {
        MoneyProgressBar->SetPercent(FMath::Clamp(InPercent, 0.0f, 1.0f));
    }
    else
    {
        LOGV(Error, TEXT("MoneyProgressBar is nullptr!"));
    }
}

void UPlayerStatusWidget::PlayNextPhaseAnim(int32 NextPhaseNumber)
{
    if (Cast<AADTutorialGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        return;
    }

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

void UPlayerStatusWidget::SetSpearGunTypeImage(int8 TypeNum)
{
    SpearGunTypeImage->SetBrushFromTexture(SpearGunTypeImages[TypeNum], true);
}

void UPlayerStatusWidget::OnChangedEnvironment(bool bIsUnderwater)
{
    if (!DepthWidget) return;
    if (bIsUnderwater)
    {
        DepthWidget->PlayOpenCloseAnim(true);
        DepthWidget->SetVisibility(ESlateVisibility::Visible);
        LOGV(Warning, TEXT("Underwater Visible"));
    }
    else
    {
        DepthWidget->PlayOpenCloseAnim(false);
        FTimerHandle AnimationDelayTimerHandle;
        float AnimationDelay = DepthWidget->GetCloseAnimLength();
        GetWorld()->GetTimerManager().SetTimer(AnimationDelayTimerHandle, [this]() { 
            DepthWidget->SetVisibility(ESlateVisibility::Hidden);
            LOGV(Warning, TEXT("Ground Hidden"));
        }, AnimationDelay, false);
    }
}

void UPlayerStatusWidget::OnNextPhaseAnimFinished()
{
    LOGV(Warning, TEXT("NextPhaseAnim Finished"));

    // 나중에 테이블로 텍스트 정리할수도?
    if (CachedNextPhaseNumber > MaxPhaseNumber)
    {
        SetCurrentPhaseText(TEXT("잠수정으로 돌아가라."));
        PhaseProgressbarOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        SetCurrentPhaseText(FString::Printf(TEXT("Phase%d"), CachedNextPhaseNumber));
        PhaseProgressbarOverlay->SetVisibility(ESlateVisibility::Visible);
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

void UPlayerStatusWidget::ShowPhaseWarning(bool bShouldVisible)
{
    if(PhaseWarningWidget)
        PhaseWarningWidget->SetbShowWarning(bShouldVisible);
}

void UPlayerStatusWidget::NoticeInfo(const FString& Info, const FVector2D& Position)
{
    if (NoticeWidget && NoticeWidget->Slot)
    {
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NoticeWidget->Slot))
        {
            CanvasSlot->SetPosition(Position);
        }
        NoticeWidget->SetNoticeText(Info);
        NoticeWidget->ShowNotice();
    }
}

void UPlayerStatusWidget::SetTopName(AADPlayerState* PS, int32 MinedAmount)
{
    TopName->SetText(FText::FromString(PS->GetPlayerNickname()));
    TopNameCopy->SetText(FText::FromString(PS->GetPlayerNickname()));

    FString TopAmountString = FString::Printf(TEXT("%dCr"), MinedAmount);
    TopAmount->SetText(FText::FromString(TopAmountString));
    PlayAnimation(ChangeTopPlayer);
}

void UPlayerStatusWidget::SetTopNameEmpty()
{
    TopName->SetText(FText::FromString(TEXT("")));
    TopNameCopy->SetText(FText::FromString(TEXT("")));
    TopAmount->SetText(FText::FromString(TEXT("")));
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

void UPlayerStatusWidget::SetMaxPhaseNumber(int32 NewMaxPhaseNumber)
{
    MaxPhaseNumber = NewMaxPhaseNumber;
}

