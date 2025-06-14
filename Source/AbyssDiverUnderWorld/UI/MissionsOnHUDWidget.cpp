#include "UI/MissionsOnHUDWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "UI/MissionEntryOnHUDWidget.h"
#include "DataRow/MissionDataRow/MissionBaseRow.h"
#include "Subsystems/MissionSubsystem.h"
#include "Framework/ADInGameState.h"

#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

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

	FActivatedMissionInfoList& MissionList = GetInGameState()->GetActivatedMissionList();

	MissionList.OnMissionInfosChangedDelegate.RemoveAll(this);
	MissionList.OnMissionInfosChangedDelegate.AddUObject(this, &UMissionsOnHUDWidget::OnMissionChanged);

	MissionList.OnMissionInfosRemovedDelegate.RemoveAll(this);
	MissionList.OnMissionInfosRemovedDelegate.AddUObject(this, &UMissionsOnHUDWidget::OnMissionRemoved);
	LOGV(Log, TEXT("DelegateBound"));

	GetInGameState()->OnMissionListRefreshedDelegate.RemoveAll(this);
	GetInGameState()->OnMissionListRefreshedDelegate.AddUObject(this, &UMissionsOnHUDWidget::Refresh);
	Refresh();
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
	MissionListBox->AddChild(Entry);
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
