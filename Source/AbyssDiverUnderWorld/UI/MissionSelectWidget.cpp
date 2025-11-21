#include "UI/MissionSelectWidget.h"
#include "UI/MissionEntryWidget.h"
#include "UI/MissionData.h"
#include "UI/ToggleWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/ADGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Subsystems/MissionSubsystem.h"

void UMissionSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Start && Button_Start->OnClicked.IsBound() == false)
    {
        Button_Start->OnReleased.AddDynamic(this, &UMissionSelectWidget::OnStartButtonClicked);
        Button_Start->SetIsEnabled(true);

        Button_MissionReset->OnReleased.AddDynamic(this, &UMissionSelectWidget::OnMissionResetButtonClicked);
        Button_MissionReset->SetIsEnabled(true); 
    }

    UpdateMissionList(0, false);

    WarningBorder->SetVisibility(ESlateVisibility::Hidden);

    bIsMissionGained = false;
}

void UMissionSelectWidget::AddMissionEntry(const FMissionData& Data)
{
    if (!MissionEntryClass || !ScrollBox_MissionList) return;

    UMissionEntryWidget* Entry = CreateWidget<UMissionEntryWidget>(GetWorld(), MissionEntryClass);
    if (Entry)
    {
        Entry->Setup(Data);
        Entry->OnMissionClicked.AddDynamic(this, &UMissionSelectWidget::OnMissionClicked);
        ScrollBox_MissionList->AddChild(Entry);
    }
}

void UMissionSelectWidget::UpdateEntrys()
{
    // 미션 선택 여부에 따라 시각적 갱신
    for (UWidget* Widget : ScrollBox_MissionList->GetAllChildren())
    {
        if (UMissionEntryWidget* Entry = Cast<UMissionEntryWidget>(Widget))
        {
            const bool bIsEntrySelected = SelectedMissions.ContainsByPredicate([&](const FMissionData& Item)
                {
                    return Item.Title == Entry->GetMissionData().Title;
                });
            Entry->SetSelected(bIsEntrySelected);
        }
    }
}

void UMissionSelectWidget::UpdateMissionList(int8 CurrentLevelIndex, bool bIsUnlock)
{
    if (CurrentLevelIndex == 0) return;
    ScrollBox_MissionList->ClearChildren();
    if (bIsUnlock)
    {
        LockImage->SetVisibility(ESlateVisibility::Hidden);
        TouchDisplay->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        LockImage->SetVisibility(ESlateVisibility::Visible);
        TouchDisplay->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    const TArray<FMissionData>& Missions = GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetMissionDataForUI();
    Algo::Sort(Missions, [](const FMissionData& A, const FMissionData& B)
        {
            return A.Stage < B.Stage;
        });
    for (const FMissionData& Mission : Missions)
    {
        if (Mission.Stage == CurrentLevelIndex)
        {
            AddMissionEntry(Mission);
        }
    }
}

void UMissionSelectWidget::UpdateSelectedMissionBox()
{
    if (!SelectedMissionsBox) return;

    SelectedMissionsBox->ClearChildren();
    for (const FMissionData& Mission : SelectedMissions)
    {
        UTextBlock* NewTextBlock = NewObject<UTextBlock>(this, UTextBlock::StaticClass());

        FText TitleText = FText::FromString(Mission.Title);
        NewTextBlock->SetText(TitleText);
        UVerticalBoxSlot* VerticalSlot = SelectedMissionsBox->AddChildToVerticalBox(NewTextBlock);
        if (VerticalSlot)
        {
            VerticalSlot->SetSize(ESlateSizeRule::Fill);
            VerticalSlot->SetHorizontalAlignment(HAlign_Center); // (선택) 가로로도 채우기
            VerticalSlot->SetVerticalAlignment(VAlign_Center);
        }
    }
}

void UMissionSelectWidget::OnMissionClicked(const FMissionData& Data, bool bSelected)
{
    if (bSelected)
    {
        if (SelectedMissions.Num() < 3 && !bIsMissionGained)
        {
            SelectedMissions.Add(Data);
        }
    }
    else
    {
        SelectedMissions.RemoveAll([&](const FMissionData& Item)
            {
                return Item.Title == Data.Title;
            });
    }
    UpdateSelectedMissionBox();
    UpdateEntrys();
}

void UMissionSelectWidget::OnStartButtonClicked()
{
    if (!SelectedMissions.IsEmpty())
    {
        if (OnStartButtonClickedDelegate.IsBound())
            OnStartButtonClickedDelegate.Broadcast(SelectedMissions);
        bIsMissionGained = true;

        SelectedMissions.Empty();
        UpdateEntrys();
        UpdateSelectedMissionBox();
    }
    else
    {
        
        FText WarningMessage = !bIsMissionGained ? FText::FromString(TEXT("미션을 골라주세요.")) :FText::FromString(TEXT("이미 미션이 지급되었습니다."));

        WarningText->SetText(WarningMessage);
        WarningBorder->SetVisibility(ESlateVisibility::Visible);

        FTimerHandle DeleteTimerHanle;
        float DeleteDelay = 1.0f;
        GetWorld()->GetTimerManager().SetTimer(DeleteTimerHanle,
            FTimerDelegate::CreateLambda([this]()
            {
                WarningBorder->SetVisibility(ESlateVisibility::Hidden);
            }), DeleteDelay, false);
    }
}

void UMissionSelectWidget::OnMissionResetButtonClicked()
{
    OnMisionResetButtonClickedDelegate.ExecuteIfBound();
    bIsMissionGained = false;
}
