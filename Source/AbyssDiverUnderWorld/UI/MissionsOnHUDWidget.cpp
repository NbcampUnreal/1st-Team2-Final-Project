#include "UI/MissionsOnHUDWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "UI/MissionEntryOnHUDWidget.h"
#include "DataRow/MissionDataRow/MissionBaseRow.h"
#include "Subsystems/MissionSubsystem.h"
#include "Framework/ADInGameState.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Missions/MissionManagerComponent.h"

void UMissionsOnHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitWiget();
}

void UMissionsOnHUDWidget::InitWiget()
{
	if (GetInGameState() == nullptr)
	{
		return;
	}

	if (AGameStateBase* GS = UGameplayStatics::GetGameState(this))
	{
		MissionManager = GS->FindComponentByClass<UMissionManagerComponent>();
	}
	if (!MissionManager) { return; }

	// 2) Manager 레벨 이벤트 바인딩 (한 곳에만 바인딩)
	MissionManager->OnMissionStatesUpdated.RemoveAll(this);
	MissionManager->OnMissionStatesUpdated.AddDynamic(this, &ThisClass::HandleStatesUpdated);

	MissionManager->OnMissionCompletedUI.RemoveAll(this);
	MissionManager->OnMissionCompletedUI.AddDynamic(this, &ThisClass::HandleMissionCompletedUI);

	// 3) 최초 1회 전체 갱신
	// 3-1) 스냅샷 빌드 함수가 있으면 사용
	TArray<FMissionRuntimeState> States;
	MissionManager->BuildMissionStateSnapshot(States);
	HandleStatesUpdated(States);
}

void UMissionsOnHUDWidget::SetVisible(bool bShouldVisible, int32 ElementIndex)
{
	UMissionEntryOnHUDWidget* Element = Cast<UMissionEntryOnHUDWidget>(MissionListBox->GetChildAt(ElementIndex));
	if (Element == nullptr)
	{
		LOGV(Error, TEXT("Element == nullptr"));
		return;
	}

	Element->SetVisible(bShouldVisible);
}

void UMissionsOnHUDWidget::UpdateMission(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress)
{
	if (GetMissionSubsystem() == nullptr)
	{
		return;
	}

	if (GetInGameState() == nullptr)
	{
		return;
	}

	const FMissionBaseRow* MissionData = MissionSubsystem->GetMissionData(MissionType, MissionIndex);

	int8* WidgetEntryIndex = ContainedMissions.Find({ MissionType, MissionIndex });
	int8 ActualIndex = INDEX_NONE;
	if (WidgetEntryIndex)
	{
		ActualIndex = *WidgetEntryIndex;
	}
	else
	{
		CreateAndAddEntry(MissionData->MissionImage);
		ActualIndex = MissionListBox->GetChildrenCount() - 1;
		ContainedMissions.Add({ MissionType, MissionIndex }, ActualIndex);
	}

	ChangeImage(MissionData->MissionImage, ActualIndex);

	UpdateMissionEntryColor(InGameState->GetActivatedMissionList().MissionInfoList[ActualIndex].bIsCompleted, ActualIndex);
}

void UMissionsOnHUDWidget::Refresh()
{
	int32 ElementCount = GetElementCount();
	for (int32 i = 0; i < ElementCount; ++i)
	{
		MissionListBox->RemoveChildAt(ElementCount - i - 1);
	}

	ContainedMissions.Reset();
	
	const TArray<FActivatedMissionInfo>& Missions = InGameState->GetActivatedMissionList().MissionInfoList;
	for (const FActivatedMissionInfo& Mission : Missions)
	{
		EMissionType MissionType = Mission.MissionType;
		uint8 MissionIndex = Mission.MissionIndex;
		int8 Progress = Mission.CurrentProgress;
		UpdateMission(MissionType, MissionIndex, Progress);
	}
}

void UMissionsOnHUDWidget::CreateAndAddEntry(UTexture2D* Image)
{
	UMissionEntryOnHUDWidget* Entry = CreateWidget<UMissionEntryOnHUDWidget>(this, MissionEntryClass);
	check(Entry);
	Entry->ChangeImage(Image);
	UVerticalBoxSlot* VerticalSlot = MissionListBox->AddChildToVerticalBox(Entry);
	if (VerticalSlot)
	{
		VerticalSlot->SetSize(ESlateSizeRule::Fill);
		VerticalSlot->SetHorizontalAlignment(HAlign_Center); // (선택) 가로로도 채우기
		VerticalSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UMissionsOnHUDWidget::ChangeImage(UTexture2D* Image, int32 ElementIndex)
{
	UMissionEntryOnHUDWidget* Element = Cast<UMissionEntryOnHUDWidget>(MissionListBox->GetChildAt(ElementIndex));
	if (Element == nullptr)
	{
		LOGV(Error, TEXT("Element == nullptr"));
		return;
	}

	Element->ChangeImage(Image);
}

void UMissionsOnHUDWidget::UpdateMissionEntryColor(bool bIsMissionCompleted, int32 ElementIndex)
{
	UMissionEntryOnHUDWidget* Element = Cast<UMissionEntryOnHUDWidget>(MissionListBox->GetChildAt(ElementIndex));
	if (Element == nullptr)
	{
		LOGV(Error, TEXT("Element == nullptr"));
		return;
	}

	Element->UpdateMissionEntryColor(bIsMissionCompleted);
}

void UMissionsOnHUDWidget::OnMissionChanged(int32 ChangedIndex, const FActivatedMissionInfoList& ChangedValue)
{
	LOGV(Log, TEXT("UpdatingMission.."));
	const FActivatedMissionInfo& MissionInfo = ChangedValue.MissionInfoList[ChangedIndex];
	UpdateMission(MissionInfo.MissionType, MissionInfo.MissionIndex, MissionInfo.CurrentProgress);
}

void UMissionsOnHUDWidget::OnMissionRemoved(int32 RemovedIndex, const FActivatedMissionInfoList& RemovedValue)
{
	LOGV(Log, TEXT("RemovingMission.."));
}

void UMissionsOnHUDWidget::HandleStatesUpdated(const TArray<FMissionRuntimeState>& States)
{
	const int32 ElementCount = GetElementCount();
	for (int32 i = 0; i < ElementCount; ++i)
	{
		MissionListBox->RemoveChildAt(ElementCount - i - 1);
	}
	ContainedMissions.Reset();

	// 제목/이미지 룩업은 기존 Subsystem 로직 유지
	UMissionSubsystem* Subsys = GetMissionSubsystem();

	for (const FMissionRuntimeState& S : States)
	{
		// 1) DataRow 룩업(제목/설명/아이콘)
		FString TitleStr, DescStr;
		UTexture2D* IconTex = nullptr;

		if (Subsys)
		{
			const FMissionBaseRow* Row = nullptr;
			switch (S.MissionType)
			{
			case EMissionType::AggroTrigger:   Row = Subsys->GetAggroTriggerMissionData((EAggroTriggerMission)S.MissionIndex); break;
			case EMissionType::Interaction:    Row = Subsys->GetInteractionMissionData((EInteractionMission)S.MissionIndex);   break;
			case EMissionType::ItemCollection: Row = Subsys->GetItemCollectMissionData((EItemCollectMission)S.MissionIndex);   break;
			case EMissionType::ItemUse:        Row = Subsys->GetItemUseMissionData((EItemUseMission)S.MissionIndex);           break;
			case EMissionType::KillMonster:    Row = Subsys->GetKillMonsterMissionData((EKillMonsterMission)S.MissionIndex);   break;
			default: break;
			}
			if (Row)
			{
				TitleStr = Row->MissionName;
				DescStr = Row->MissionDescription;    // DataRow에 설명 필드가 있다고 가정
				IconTex = Row->MissionImage;
			}
		}

		// 2) 엔트리 생성 (기존 CreateAndAddEntry()가 위젯 리턴한다면 그걸 사용)
		UMissionEntryOnHUDWidget* Entry = CreateWidget<UMissionEntryOnHUDWidget>(this, MissionEntryClass);
		if (!Entry) continue;

		Entry->ChangeImage(IconTex);
		Entry->SetTitle(FText::FromString(TitleStr));
		Entry->SetDescription(FText::FromString(DescStr));
		Entry->SetProgress(S.Current, S.Goal);
		Entry->SetCompleted(S.bCompleted);

		UVerticalBoxSlot* BoxSlot = MissionListBox->AddChildToVerticalBox(Entry);
		if (BoxSlot)
		{
			BoxSlot->SetSize(ESlateSizeRule::Automatic);
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			BoxSlot->SetVerticalAlignment(VAlign_Top);
		}

		// 인덱스 맵 유지(완료 즉시 처리용)
		const int8 ActualIndex = (int8)(MissionListBox->GetChildrenCount() - 1);
		ContainedMissions.Add({ S.MissionType, (uint8)S.MissionIndex }, ActualIndex);
	}
}

void UMissionsOnHUDWidget::HandleMissionCompletedUI(EMissionType Type, int32 Index)
{
	if (int8* P = ContainedMissions.Find({ Type, (uint8)Index }))
	{
		if (auto* Entry = Cast<UMissionEntryOnHUDWidget>(MissionListBox->GetChildAt(*P)))
		{
			Entry->SetCompleted(true);
			// (선택) 여기서 토스트/사운드/진동 재생
		}
	}
}

int32 UMissionsOnHUDWidget::GetElementCount() const
{
	return MissionListBox->GetChildrenCount();
}

UMissionSubsystem* UMissionsOnHUDWidget::GetMissionSubsystem()
{
	if (::IsValid(MissionSubsystem))
	{
		return MissionSubsystem;
	}

	UGameInstance* GI = GetGameInstance();
	if (GI == nullptr)
	{
		LOGV(Error, TEXT("GI == nullptr"));
		return nullptr;
	}

	MissionSubsystem = GI->GetSubsystem<UMissionSubsystem>();

	return MissionSubsystem;
}

AADInGameState* UMissionsOnHUDWidget::GetInGameState()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsValidLowLevel() == false || World->bIsTearingDown)
	{
		LOGV(Error, TEXT("World is Invalid"));
		return nullptr;
	}
	
	InGameState = World->GetGameState<AADInGameState>();

	return InGameState;
}
