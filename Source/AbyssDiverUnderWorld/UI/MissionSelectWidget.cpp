#include "UI/MissionSelectWidget.h"
#include "UI/MissionEntryWidget.h"
#include "UI/MissionData.h"
#include "UI/AllInventoryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Framework/ADGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UnderwaterCharacter.h"

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

    if (Button_Start)
    {
        Button_Start->OnClicked.AddDynamic(this, &UMissionSelectWidget::OnStartButtonClicked);
        Button_Start->SetIsEnabled(true); // 항상 활성화
    }

    AllMissions = {
        {TEXT("산소 캡슐 회수"), 1, true, TEXT("")},
        {TEXT("잃어버린 장비 찾기"), 1, true, TEXT("")},
        {TEXT("깊은 수중 탐사"), 2, true, TEXT("")},
        {TEXT("???"), 3, false, TEXT("얕은 해류 클리어 시 해금")},
        {TEXT("???"), 3, false, TEXT("얕은 해류 클리어 시 해금")}
    };

    for (const FMissionData& Mission : AllMissions)
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
    if (UADGameInstance* GI = Cast<UADGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->SelectedMissions = SelectedMissions;
        UE_LOG(LogTemp, Warning, TEXT("✅ [MissionSelectWidget] 선택된 미션 수: %d"), SelectedMissions.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [MissionSelectWidget] GameInstance cast 실패"));
    }

    RemoveFromParent();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        APawn* Pawn = PC->GetPawn();
        if (AUnderwaterCharacter* UWCharacter = Cast<AUnderwaterCharacter>(Pawn))
        {
            if (UWCharacter->AllInventoryWidgetClass)
            {
                UWCharacter->InventoryWidgetInstance = CreateWidget<UAllInventoryWidget>(GetWorld(), UWCharacter->AllInventoryWidgetClass);

                if (UWCharacter->InventoryWidgetInstance)
                {
                    UE_LOG(LogTemp, Warning, TEXT("✅ [MissionSelectWidget] 인벤토리 위젯 생성 성공"));

                    // ❌ 바로 AddToViewport 하지 말기!
                    // ❌ UWCharacter->InventoryWidgetInstance->AddToViewport();

                    UWCharacter->InventoryWidgetInstance->RefreshMissionList(); // ✅ 미리 세팅만
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ [MissionSelectWidget] AllInventoryWidgetClass가 설정되지 않음"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ [MissionSelectWidget] UWCharacter 캐스팅 실패"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("🎮 [MissionSelectWidget] 미션 %d개 선택됨. 게임 시작."), SelectedMissions.Num());
}

