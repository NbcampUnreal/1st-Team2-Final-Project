#include "UI/MissionSelectWidget.h"
#include "UI/MissionEntryWidget.h"
#include "UI/MissionData.h"
#include "UI/ToggleWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/ADGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Subsystems/MissionSubsystem.h"
#include "Framework/ADPlayerController.h"

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

    UpdateMissionList(0);

    WarningBorder->SetVisibility(ESlateVisibility::Hidden);

    bIsMissionGained = false;
    UE_LOG(LogTemp, Warning, TEXT("[ Mission Entry] Construct: %s, Parent=%s, Vis=%d, Opacity=%.2f"),
        *GetPathName(),
        GetParent() ? *GetParent()->GetPathName() : TEXT("None"),
        (int)GetVisibility(), GetRenderOpacity());
    UpdateMissionList_NoFilter();
}

void UMissionSelectWidget::AddMissionEntry(const FMissionData& Data)
{
    if (!MissionEntryClass || !ScrollBox_MissionList) return;

    UMissionEntryWidget* Entry = CreateWidget<UMissionEntryWidget>(this, MissionEntryClass);
    if (Entry)
    {
        Entry->Setup(Data);
        Entry->OnMissionClicked.AddDynamic(this, &UMissionSelectWidget::OnMissionClicked);
        ScrollBox_MissionList->AddChild(Entry);
        LOG(TEXT(""));
        Entry->ForceLayoutPrepass();
        ScrollBox_MissionList->InvalidateLayoutAndVolatility();
        ScrollBox_MissionList->ScrollToStart();
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

void UMissionSelectWidget::UpdateMissionList(int8 CurrentLevelIndex)
{
    if (!ScrollBox_MissionList) return;

    ScrollBox_MissionList->ClearChildren();

    const TArray<FMissionData>& MissionsRef =
        GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetMissionDataForUI();

    TArray<FMissionData> Missions = MissionsRef;     // 복사
    Algo::Sort(Missions, [](const FMissionData& A, const FMissionData& B) { return A.Stage < B.Stage; });

    for (const FMissionData& Mission : Missions)
    {

        AddMissionEntry(Mission);
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
        if (SelectedMissions.IsEmpty()) { /* 경고 UI */ return; }

        if (auto* PC = GetOwningPlayer())
        {
            if (auto* ADPC = Cast<AADPlayerController>(PC))
            {
                ADPC->S_SubmitSelectedMissions(SelectedMissions);
            }
        }


        // UI 정리
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

void UMissionSelectWidget::UpdateMissionList_NoFilter()
{
    auto* GI = GetGameInstance();
    auto* Sub = GI ? GI->GetSubsystem<UMissionSubsystem>() : nullptr;
    if (!ensure(Sub)) return;
    if (!ensure(ScrollBox_MissionList)) { UE_LOG(LogTemp, Warning, TEXT("[MSel] ScrollBox is null")); return; }
    if (!ensure(MissionEntryClass)) { UE_LOG(LogTemp, Warning, TEXT("[MSel] MissionEntryClass is null")); return; }

    const auto& List = Sub->GetMissionDataForUI();
    UE_LOG(LogTemp, Warning, TEXT("[MSel] Will render %d entries"), List.Num());

    ScrollBox_MissionList->ClearChildren();

    for (const FMissionData& Data : List)
    {
        UMissionEntryWidget* Entry = CreateWidget<UMissionEntryWidget>(this, MissionEntryClass);
        if (!Entry) { UE_LOG(LogTemp, Warning, TEXT("[MSel] CreateWidget failed")); continue; }

        Entry->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // ★ 항상 보이게
        Entry->SetRenderOpacity(1.0f);                                 // ★ 불투명 강제
        Entry->Setup(Data);                                      // 너의 세터

        UPanelSlot* asSlot = ScrollBox_MissionList->AddChild(Entry);
        UE_LOG(LogTemp, Warning, TEXT("[MSel] + Added %s to %s (children=%d)"),
            *Entry->GetPathName(), *ScrollBox_MissionList->GetPathName(),
            ScrollBox_MissionList->GetChildrenCount());
    }
}