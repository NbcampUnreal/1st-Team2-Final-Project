#include "UI/MissionSelectWidget.h"
#include "UI/MissionEntryWidget.h"
#include "UI/MissionData.h"
#include "UI/ToggleWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Framework/ADGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Subsystems/MissionSubsystem.h"

void UMissionSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    if (Button_Start && Button_Start->OnClicked.IsBound() == false)
    {
        Button_Start->OnClicked.AddDynamic(this, &UMissionSelectWidget::OnStartButtonClicked);
        Button_Start->SetIsEnabled(true); // 항상 활성화
    }

    /*AllMissions = {
        {TEXT("산소 캡슐 회수"), 1, true, TEXT("")},
        {TEXT("잃어버린 장비 찾기"), 1, true, TEXT("")},
        {TEXT("깊은 수중 탐사"), 2, true, TEXT("")},
        {TEXT("???"), 3, false, TEXT("얕은 해류 클리어 시 해금")},
        {TEXT("???"), 3, false, TEXT("얕은 해류 클리어 시 해금")}
    };*/

    ScrollBox_MissionList->ClearChildren();
    const TSet<FMissionData>& Missions = GetGameInstance()->GetSubsystem<UMissionSubsystem>()->GetMissionDataForUI();

    for (const FMissionData& Mission : Missions)
    {
        AddMissionEntry(Mission);
    }
}

void UMissionSelectWidget::AddMissionEntry(const FMissionData& Data)
{
    if (!MissionEntryClass || !ScrollBox_MissionList) return;

    UMissionEntryWidget* Entry = CreateWidget<UMissionEntryWidget>(GetWorld(), MissionEntryClass);
    if (Entry)
    {
        Entry->Setup(Data);

        if (Data.bIsUnlocked)
        {
            Entry->OnMissionClicked.AddDynamic(this, &UMissionSelectWidget::OnMissionClicked);
        }

        ScrollBox_MissionList->AddChild(Entry);
    }
}

void UMissionSelectWidget::OnMissionClicked(const FMissionData& Data, bool bSelected)
{
    if (bSelected)
    {
        if (!SelectedMissions.ContainsByPredicate([&](const FMissionData& Item) { return Item.Title == Data.Title; }))
        {
            if (SelectedMissions.Num() < 3)
            {
                SelectedMissions.Add(Data);
            }
        }
    }
    else
    {
        SelectedMissions.RemoveAll([&](const FMissionData& Item)
            {
                return Item.Title == Data.Title;
            });
    }

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

void UMissionSelectWidget::OnStartButtonClicked()
{
    RemoveFromParent();  // 👉 미션 선택 UI 닫기

    OnStartButtonClickedDelegate.ExecuteIfBound(SelectedMissions);

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>())
        {
            PS->SetSelectedMissions(SelectedMissions);
            UE_LOG(LogTemp, Warning, TEXT("✅ [MissionSelectWidget] 선택된 미션 수: %d"), SelectedMissions.Num());
        }

        // 입력 모드 원복
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        // ✅ 미션 리스트 위젯 생성 및 화면에 추가
        if (WBP_SelectedMissionListClass) // UPROPERTY로 받은 위젯 클래스
        {
            USelectedMissionListWidget* MissionListWidget = CreateWidget<USelectedMissionListWidget>(GetWorld(), WBP_SelectedMissionListClass);
            if (MissionListWidget)
            {
                MissionListWidget->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("✅ 선택된 미션 리스트 위젯 AddToViewport 완료"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ 선택된 미션 리스트 위젯 생성 실패"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ WBP_SelectedMissionListClass가 설정되지 않음"));
        }
    }
}
