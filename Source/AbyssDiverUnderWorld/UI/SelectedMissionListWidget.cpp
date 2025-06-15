#include "UI/SelectedMissionListWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"

#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USelectedMissionListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!VerticalBox_MissionList)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ VerticalBox_MissionList 바인딩 실패"));
        return;
    }

    Refresh();
}

void USelectedMissionListWidget::AddElement(const FString& ElementText)
{
    //미션 구조체 추가

    UTextBlock* MissionText = NewObject<UTextBlock>(this);
    MissionText->SetText(FText::FromString(ElementText));
    MissionText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 17)));
    MissionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

    UVerticalBoxSlot* VerticalSlot = VerticalBox_MissionList->AddChildToVerticalBox(MissionText);
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

    UTextBlock* Element = CastChecked<UTextBlock>(VerticalBox_MissionList->GetChildAt(Index));

    Element->SetText(FText::FromString(NewText));
    LOGV(Warning, TEXT("%s"), *NewText);
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
        break;
    case EMissionType::Interaction:
        NewElementText = MissionSubsystem->GetInteractionMissionData((EInteractionMission)MissionIndex)->MissionName;
        break;
    case EMissionType::ItemCollection:
        NewElementText = MissionSubsystem->GetItemCollectMissionData((EItemCollectMission)MissionIndex)->MissionName;
        break;
    case EMissionType::ItemUse:
        NewElementText = MissionSubsystem->GetItemUseMissionData((EItemUseMission)MissionIndex)->MissionName;
        break;
    case EMissionType::KillMonster:
        NewElementText = MissionSubsystem->GetKillMonsterMissionData((EKillMonsterMission)MissionIndex)->MissionName;
        break;
    default:
        check(false);
        return;
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

    for (int32 i = 0; i < MissionInfos.Num(); ++i)
    {
        AddOrModifyElement(i, MissionList);
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
