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
	GetWorld()->GetTimerManager().SetTimer(InitRetryHandle, this, &UMissionsOnHUDWidget::TryBindManager, 0.25f, true);
}

void UMissionsOnHUDWidget::NativeDestruct()
{
	if (MissionManager)
	{
		MissionManager->OnMissionStatesUpdated.RemoveAll(this);
		MissionManager->OnMissionCompletedUI.RemoveAll(this);
	}
	GetWorld()->GetTimerManager().ClearTimer(InitRetryHandle);
	Super::NativeDestruct();
}

void UMissionsOnHUDWidget::InitWidget()
{
	if (!MissionManager)
	{
		LOG(TEXT("[MissionsHUD] InitWidget - MissionManager nullptr"));
		return;
	}

	LOG(TEXT("[MissionsHUD] InitWidget - Delegate binding start"));

	// 중복 바인딩 방지
	MissionManager->OnMissionStatesUpdated.RemoveAll(this);
	MissionManager->OnMissionCompletedUI.RemoveAll(this);

	MissionManager->OnMissionStatesUpdated.AddDynamic(this, &ThisClass::HandleStatesUpdated);
	MissionManager->OnMissionCompletedUI.AddDynamic(this, &ThisClass::HandleMissionCompletedUI);

	// 최초 1회 스냅샷으로 그리고 시작
	TArray<FMissionRuntimeState> States;
	MissionManager->BuildMissionStateSnapshot(States);

	LOG(TEXT("[MissionsHUD] Snapshot ready - %d states"), States.Num());
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

void UMissionsOnHUDWidget::UpdateMission(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress, int32 Goal, bool bCompleted)
{
	if (!MissionListBox) return;

	if (GetMissionSubsystem() == nullptr)
	{
		return;
	}

	if (GetInGameState() == nullptr)
	{
		return;
	}
	TPair<EMissionType, uint8> Key(MissionType, MissionIndex);
	int8* FoundSlotPtr = MissionEntryMap.Find(Key);
	int8 SlotIndex = FoundSlotPtr ? *FoundSlotPtr : -1;

	// 1) 없으면 만들고 붙이기
	if (SlotIndex == -1)
	{
		// 새로 추가
		SlotIndex = CreateAndAttachEntry(MissionType, MissionIndex);
		if (SlotIndex == -1)
		{
			LOG(TEXT("[MissionsHUD] UpdateMission - Failed to create entry (T=%d,Idx=%d)"),
				(int32)MissionType, (int32)MissionIndex);
			return;
		}

	}
	// 2) 엔트리 가져와서 값 갱신
	if (UMissionEntryOnHUDWidget* Entry = GetEntryBySlotIndex(SlotIndex))
	{
		Entry->SetProgress(CurrentProgress, Goal);
		Entry->SetCompleted(bCompleted);

		LOG(TEXT("[MissionsHUD]  * Update slot %d (Cur=%d / Goal=%d / Done=%d)"),
			(int32)SlotIndex, CurrentProgress, Goal, bCompleted);
	}

}

void UMissionsOnHUDWidget::MissionStatesRefresh()
{
	if (!MissionManager) return;

	TArray<FMissionRuntimeState> States;
	MissionManager->BuildMissionStateSnapshot(States);

	LOG(TEXT("[MissionsHUD] MissionStates - %d states"), States.Num());
	HandleStatesUpdated(States);
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

void UMissionsOnHUDWidget::TryBindManager()
{
	if (!MissionManager)
	{
		if (AADInGameState* GS = GetInGameState())
		{
			MissionManager = GS->FindComponentByClass<UMissionManagerComponent>();
			LOG(TEXT("[MissionsHUD] TryBindManager - GameState:%s  Manager:%s"),
				*GetNameSafe(GS), *GetNameSafe(MissionManager));
		}
	}
	if (MissionManager)
	{
		GetWorld()->GetTimerManager().ClearTimer(InitRetryHandle);
		InitWidget();
	}
}


void UMissionsOnHUDWidget::HandleStatesUpdated(const TArray<FMissionRuntimeState>& States)
{
	LOG(TEXT("[MissionsHUD] HandleStatesUpdated - %d states"), States.Num());
	if (!MissionListBox)
	{
		LOG(TEXT("[MissionsHUD] MissionListBox is nullptr"));
		return;
	}

	if (!MissionEntryClass)
	{
		LOG(TEXT("[MissionsHUD] MissionEntryClass is nullptr"));
		return;
	}
	//  인덱스 맵 구조 특성상 안전하게 전체 재빌드(인덱스 꼬임 방지)
	MissionListBox->ClearChildren();
	MissionEntryMap.Empty();

	// (선택) 표시 순서 보장 원하면 여기서 정렬
	// TArray<FMissionRuntimeState> Sorted = States; ... 정렬 ...

	for (const FMissionRuntimeState& S : States)
	{
		LOG(TEXT("[MissionsHUD]  → Mission T=%d Idx=%d Cur=%d Goal=%d Done=%d"),
			(int32)S.MissionType, (int32)S.MissionIndex, S.Current, S.Goal, S.bCompleted);

		UpdateMission(static_cast<EMissionType>(S.MissionType), (uint8)S.MissionIndex, S.Current, S.Goal, S.bCompleted);
	}
}

void UMissionsOnHUDWidget::HandleMissionCompletedUI(EMissionType MissionType, int32 MissionIndex)
{
	const TPair<EMissionType, uint8> Key(static_cast<EMissionType>(MissionType), (uint8)MissionIndex);
	if (int8* Found = MissionEntryMap.Find(Key))
	{
		if (UMissionEntryOnHUDWidget* Entry = GetEntryBySlotIndex(*Found))
		{
			Entry->SetCompleted(true);
			// BP 쪽 애니메이션을 노출했다면:
			// Entry->PlayCompleteEffect();
		}
	}
}

UMissionEntryOnHUDWidget* UMissionsOnHUDWidget::GetEntryBySlotIndex(int8 SlotIndex) const
{
	if (!MissionListBox)
		return nullptr;
	const int32 Count = MissionListBox->GetChildrenCount();
	if (SlotIndex < 0 || SlotIndex >= Count)
		return nullptr;

	return Cast<UMissionEntryOnHUDWidget>(MissionListBox->GetChildAt(SlotIndex));
}

int8 UMissionsOnHUDWidget::CreateAndAttachEntry(EMissionType Type, uint8 MissionIndex)
{
	if (!MissionListBox || !MissionEntryClass)
		return -1;

	UMissionEntryOnHUDWidget* Entry = CreateWidget<UMissionEntryOnHUDWidget>(this, MissionEntryClass);
	if (!Entry)
	{
		LOG(TEXT("[MissionsHUD] CreateAndAttachEntry - CreateWidget failed"));
		return -1;
	}

	if (UVerticalBoxSlot* BoxSlot = MissionListBox->AddChildToVerticalBox(Entry))
	{
		BoxSlot->SetSize(ESlateSizeRule::Automatic);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		BoxSlot->SetVerticalAlignment(VAlign_Top);
	}

	const int8 NewSlotIndex = static_cast<int8>(MissionListBox->GetChildrenCount() - 1);

	LOG(TEXT("[MissionsHUD]  + Entry created (T=%d,Idx=%d) → Slot=%d"),
		(int32)Type, (int32)MissionIndex, (int32)NewSlotIndex);

	FillEntryStaticTexts(Entry, Type, MissionIndex);

	Entry->UpdateMissionEntryColor(false);
	Entry->SetVisible(true);

	MissionEntryMap.Add(TPair<EMissionType, uint8>(Type, MissionIndex), NewSlotIndex);
	return NewSlotIndex;
}

void UMissionsOnHUDWidget::FillEntryStaticTexts(UMissionEntryOnHUDWidget* Entry, EMissionType Type, uint8 MissionIndex)
{
	if (!Entry) return;

	if (UMissionSubsystem* Subsys = GetMissionSubsystem())
	{
		// 예) const FMissionBaseRow* Row = Subsys->FindBaseRow(Type, MissionIndex);
		if (const FMissionBaseRow* Row = Subsys->GetMissionData(Type, MissionIndex))
		{

			Entry->SetTitle(FText::FromString(Row->MissionName));
			Entry->SetDescription(FText::FromString(Row->MissionDescription));
			// 아이콘/색 등도 필요시 채우기
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
