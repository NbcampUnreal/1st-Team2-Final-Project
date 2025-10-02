#include "UI/SelectedMissionListWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"

#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SelectedMissionSlot.h"


void USelectedMissionListWidget::NativeConstruct()
{
    Super::NativeConstruct();


    if (!VerticalBox_MissionList)
    {
        UE_LOG(LogTemp, Error, TEXT(" VerticalBox_MissionList 바인딩 실패"));
        return;
    }


    Refresh();
}


void USelectedMissionListWidget::AddElement(const FString& ElementText)
{
    //미션 구조체 추가
    if (!SelectedMissionSlotClass) return;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->IsLocalController()) return;

    USelectedMissionSlot* SlotWidget = CreateWidget<USelectedMissionSlot>(PC, SelectedMissionSlotClass);
    SlotWidget->SetMissionTitle(ElementText);
    //UTextBlock* MissionText = NewObject<UTextBlock>(this);
    //MissionText->SetText(FText::FromString(ElementText));
    //MissionText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 17)));
    //MissionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

    UVerticalBoxSlot* VerticalSlot = VerticalBox_MissionList->AddChildToVerticalBox(SlotWidget);
    if (VerticalSlot)
    {
        VerticalSlot->SetSize(ESlateSizeRule::Fill);
        VerticalSlot->SetHorizontalAlignment(HAlign_Left); // (선택) 가로로도 채우기
        VerticalSlot->SetVerticalAlignment(VAlign_Center);
    }
    LOGV(Warning, TEXT("%s"), *ElementText);
}

void USelectedMissionListWidget::RemoveElementAt(const int32& Index)
{
    VerticalBox_MissionList->RemoveChildAt(Index);
}

void USelectedMissionListWidget::ModifyElementAt(const FString& NewText, const int32& Index)
{
    //인덱스 받아서 텍스트의 색깔을 바꾸거나 줄 긋기

    USelectedMissionSlot* MissionSlot = Cast<USelectedMissionSlot>(VerticalBox_MissionList->GetChildAt(Index));
    if (!Slot) 
    { 
        LOGV(Error, TEXT("ModifyElementAt: 인덱스 %d의 위젯이 USelectedMissionSlot이 아님"), Index); return; 
    }
    MissionSlot->SetMissionTitle(NewText);
    LOGV(Warning, TEXT("%s"), *NewText);
}

void USelectedMissionListWidget::FinishElementAt(const int8& Index)
{
    USelectedMissionSlot* Element = CastChecked<USelectedMissionSlot>(VerticalBox_MissionList->GetChildAt(Index));
    Element->OnMissionFinished();
}

void USelectedMissionListWidget::AddOrModifyElement(int32 ChangedIndex, const FActivatedMissionInfoList& ChangedValue)
{
    UMissionSubsystem* MissionSubsystem = GetGameInstance()->GetSubsystem<UMissionSubsystem>();
    if (MissionSubsystem == nullptr)
    {
        LOGV(Warning, TEXT("MissionSubsystem == nullptr"));
        return;
    }

    EMissionType MissionType = ChangedValue.MissionInfoList[ChangedIndex].MissionType;
    uint8 MissionIndex = ChangedValue.MissionInfoList[ChangedIndex].MissionIndex;
    FString NewElementText = "";

    switch (MissionType)
    {
    case EMissionType::AggroTrigger:
        NewElementText = MissionSubsystem->GetAggroTriggerMissionData((EAggroTriggerMission)MissionIndex)->MissionName;
        if (const auto* Row = MissionSubsystem->GetAggroTriggerMissionData((EAggroTriggerMission)MissionIndex))
            NewElementText = Row->MissionName;
        break;
    case EMissionType::Interaction:
        NewElementText = MissionSubsystem->GetInteractionMissionData((EInteractionMission)MissionIndex)->MissionName;
        if (const auto* Row = MissionSubsystem->GetInteractionMissionData((EInteractionMission)MissionIndex))
			NewElementText = Row->MissionName;
        break;
    case EMissionType::ItemCollection:
        NewElementText = MissionSubsystem->GetItemCollectMissionData((EItemCollectMission)MissionIndex)->MissionName;
		if (const auto* Row = MissionSubsystem->GetItemCollectMissionData((EItemCollectMission)MissionIndex))
			NewElementText = Row->MissionName;
        break;
    case EMissionType::ItemUse:
        NewElementText = MissionSubsystem->GetItemUseMissionData((EItemUseMission)MissionIndex)->MissionName;
		if (const auto* Row = MissionSubsystem->GetItemUseMissionData((EItemUseMission)MissionIndex))
			NewElementText = Row->MissionName;
        break;
    case EMissionType::KillMonster:
        NewElementText = MissionSubsystem->GetKillMonsterMissionData((EKillMonsterMission)MissionIndex)->MissionName;
        if (const auto* Row = MissionSubsystem->GetKillMonsterMissionData((EKillMonsterMission)MissionIndex))
			NewElementText = Row->MissionName;
        break;
    default:
        check(false);
        return;
    }

    if (NewElementText.IsEmpty())
    {
        LOGV(Warning, TEXT("미션 이름이 비었습니다. Type=%d, Index=%d"), (int32)MissionType, (int32)MissionIndex);
    }

    if (VerticalBox_MissionList->GetChildAt(ChangedIndex))
    {
        ModifyElementAt(NewElementText, ChangedIndex);
    }
    else
    {
        AddElement(NewElementText);
    }
}

void USelectedMissionListWidget::Refresh()
{
    AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (GS == nullptr)
    {
        LOGV(Warning, TEXT("GS == nullptr"));
        return;
    }

    FActivatedMissionInfoList& MissionList = GS->GetActivatedMissionList();

    const TArray<FActivatedMissionInfo>& MissionInfos = MissionList.MissionInfoList;
    VerticalBox_MissionList->ClearChildren();
    LOGV(Warning, TEXT("Refresh: ActivatedMissionList Count = %d"), MissionInfos.Num());

    for (int32 i = 0; i < MissionInfos.Num(); ++i)
    {
        AddOrModifyElement(i, MissionList);
    }

    if (VerticalBox_MissionList->GetChildrenCount() == 0)
    {
        LOGV(Warning, TEXT("Refresh 직후에도 자식이 0개입니다. SlotClass / MissionList를 확인하세요."));
    }

    if (GS->HasAuthority())
    {
        GS->OnMissionListRefreshedDelegate.RemoveAll(this);
        GS->OnMissionListRefreshedDelegate.AddUObject(this, &USelectedMissionListWidget::Refresh);
    }
    else
    {
        MissionList.OnMissionInfosChangedDelegate.RemoveAll(this);
        MissionList.OnMissionInfosChangedDelegate.AddUObject(this, &USelectedMissionListWidget::AddOrModifyElement);
    }
}
