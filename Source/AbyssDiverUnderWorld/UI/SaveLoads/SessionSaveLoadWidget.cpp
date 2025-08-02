#include "UI/SaveLoads/SessionSaveLoadWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "UI/SaveLoads/SavedSessionInfoPanelWidget.h"
#include "UI/SaveLoads/SavedSessionInfoWidget.h"
#include "UI/WaitForSomethingWidget.h"
#include "UI/ConfirmationDialogWidget.h"

#include "Subsystems/SaveDataSubsystem.h"
#include "SaveData/CoopData/SavedSessionInfoSaveGame.h"
#include "SaveData/CoopData/SavedSessionInfoWidgetData.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"

void USessionSaveLoadWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton->OnClicked.IsAlreadyBound(this, &USessionSaveLoadWidget::OnBackButtonClicked))
	{
		return;
	}

	BackButton->OnClicked.AddDynamic(this, &USessionSaveLoadWidget::OnBackButtonClicked);
	SaveButton->OnClicked.AddDynamic(this, &USessionSaveLoadWidget::OnSaveButtonClicked);
	LoadButton->OnClicked.AddDynamic(this, &USessionSaveLoadWidget::OnLoadButtonClicked);
	DeleteButton->OnClicked.AddDynamic(this, &USessionSaveLoadWidget::OnDeleteButtonClicked);

	ConfirmationDialogWidget->OnYesButtonClickedDelegate.AddUObject(this, &USessionSaveLoadWidget::OnYesButtonClickedWithConfirmationDialog);
	ConfirmationDialogWidget->OnNoButtonClickedDelegate.AddUObject(this, &USessionSaveLoadWidget::OnNoButtonClickedWithConfirmationDialog);

	InitWidget();
}

void USessionSaveLoadWidget::InitWidget()
{
	// ListView에 아이템 넣고 필요한 델리게이트 연결 및 필요한 초기화

	USaveDataSubsystem* SaveDataSubsystem = GetGameInstance()->GetSubsystem<USaveDataSubsystem>();
	if (SaveDataSubsystem == nullptr)
	{
		LOGV(Error, TEXT("SaveDataSubsystem is not valid"));
		return;
	}

	USavedSessionInfoSaveGame* SavedSesssionListInfo = SaveDataSubsystem->GetSavedSesssionListInfo();
	if (IsValid(SavedSesssionListInfo) == false)
	{
		LOGV(Error, TEXT("SavedSesssionListInfo is not valid"));
		return;
	}

	SessionInfoList = SavedSesssionListInfo->SavedSessionInfos.Array();

	Algo::Sort(SessionInfoList, [](const TPair<FString, FSavedSessionInfo>& A, const TPair<FString, FSavedSessionInfo>& B)
		{
			return A.Value.SavedUnixTime > B.Value.SavedUnixTime;
		});

	
	SaveInfoListView->ClearListItems();

	const int32 SessionInfoCount = SessionInfoList.Num();
	for (int32 i = 0; i < SessionInfoCount; ++i)
	{
		const TPair<FString, FSavedSessionInfo>& SessionInfo = SessionInfoList[i];
		FString FormattedTime = SessionInfo.Value.GetTimestempString();

		USavedSessionInfoWidgetData* NewData = NewObject<USavedSessionInfoWidgetData>();
		NewData->InitData(SessionInfo.Value.SaveName, FormattedTime, i);
		NewData->OnInfoWidgetUpdated.BindUObject(this, &USessionSaveLoadWidget::OnInfoWidgetUpdated);

		SaveInfoListView->AddItem(NewData);
	}

	WaitForSaveAndLoadWidget->SetVisibility(ESlateVisibility::Hidden);
	ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Hidden);

	if (CachedWidgetIndex != INDEX_NONE)
	{
		SetSessionInfoToNotClickedColor(CachedWidgetIndex);
		CachedWidgetIndex = INDEX_NONE;
	}

	SaveFimeNameInputField->SetText(FText());

	if (SaveDataSubsystem->OnSaveCompletedDelegate.IsBoundToObject(this))
	{
		return;
	}

	SaveDataSubsystem->OnSaveCompletedDelegate.AddUObject(this, &USessionSaveLoadWidget::OnSaveCompleted);
}

void USessionSaveLoadWidget::OnBackButtonClicked()
{
	// 위젯 닫기
	SetVisibility(ESlateVisibility::Hidden);
}

void USessionSaveLoadWidget::OnSaveButtonClicked()
{
	// 인풋 필드 텍스트 가져와서 저장하기 이전에 일단 파일 존재하면 덮어씌울 것인가 물어보고 기다리는 위젯 띄우기

	USaveDataSubsystem* SaveDataSubsystem = GetGameInstance()->GetSubsystem<USaveDataSubsystem>();
	if (SaveDataSubsystem == nullptr)
	{
		LOGV(Error, TEXT("SaveDataSubsystem is not valid"));
		return;
	}

	FString SaveFileName = SaveFimeNameInputField->GetText().ToString();
	if (SaveFileName.IsEmpty())
	{
		return;
	}

	FString SavedSlotName = SaveDataSubsystem->GetSavedSlotNameFromSaveName(SaveFileName);

	bool bShouldCheckIfOverwriteFile = (SavedSlotName.IsEmpty() == false);
	if (bShouldCheckIfOverwriteFile)
	{
		CurrentConfirmationStep = EConfirmationStep::Save;
		ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Visible);
		ConfirmationDialogWidget->SetConfirmationText(TEXT("Are you sure you want to overwrite the saved file?"));
	}
	else
	{
		ActivateWaitForSaveWidget();
		SaveDataSubsystem->AsyncSaveCurrentGame(SaveFileName, false);
	}
}

void USessionSaveLoadWidget::OnLoadButtonClicked()
{
	if (CachedWidgetIndex == INDEX_NONE)
	{
		return;
	}

	CurrentConfirmationStep = EConfirmationStep::Load;
	ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Visible);
	ConfirmationDialogWidget->SetConfirmationText(TEXT("Are you sure you want to load this game?"));
}

void USessionSaveLoadWidget::OnDeleteButtonClicked()
{
	if (CachedWidgetIndex == INDEX_NONE)
	{
		return;
	}

	CurrentConfirmationStep = EConfirmationStep::Delete;
	ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Visible);
	ConfirmationDialogWidget->SetConfirmationText(TEXT("Are you sure you want to delete this game?"));
}

void USessionSaveLoadWidget::OnInfoWidgetUpdated(USavedSessionInfoWidget* UpdatedWidget)
{
	if (UpdatedWidget->GetWidgetIndex() == CachedWidgetIndex)
	{
		UpdatedWidget->SetToClickedColor();
	}
	else
	{
		UpdatedWidget->SetToNotClickedColor();
	}

	if (UpdatedWidget->OnInfoWidgetClickedDelegate.IsBoundToObject(this))
	{
		return;
	}

	UpdatedWidget->OnInfoWidgetClickedDelegate.AddUObject(this, &USessionSaveLoadWidget::OnInfoWidgetClicked);
}

void USessionSaveLoadWidget::OnInfoWidgetClicked(int32 ClickedWidgetIndex)
{
	// 저장 파일 정보 목록 entry 누를 때 정보 띄우기
	if (CachedWidgetIndex != INDEX_NONE)
	{
		SetSessionInfoToNotClickedColor(CachedWidgetIndex);
	}

	CachedWidgetIndex = ClickedWidgetIndex;
	SetSessionInfoToClickedColor(CachedWidgetIndex);

	UObject* ClickedItem = SaveInfoListView->GetItemAt(CachedWidgetIndex);
	USavedSessionInfoWidget* WidgetEntryFromClickedItem = Cast<USavedSessionInfoWidget>(SaveInfoListView->GetEntryWidgetFromItem(ClickedItem));
	if (WidgetEntryFromClickedItem)
	{
		WidgetEntryFromClickedItem->SetToClickedColor();
	}

	const FSavedSessionInfo& SessionInfo = SessionInfoList[CachedWidgetIndex].Value;

	SavedSessionInfoPanel->SetSavedNameText(SessionInfo.SaveName);
	SavedSessionInfoPanel->SetSaveDateText(SessionInfo.GetTimestempString());
	SavedSessionInfoPanel->SetClearCountNumberText(SessionInfo.ClearCount);
	SavedSessionInfoPanel->SetPlayerNameTexts(SessionInfo.SavedPlayerNameTextList);
}

void USessionSaveLoadWidget::OnSaveCompleted()
{
	InitWidget();
}

void USessionSaveLoadWidget::OnYesButtonClickedWithConfirmationDialog()
{
	// Yes 버튼 눌렀을 때 상황에 따른 행동
	USaveDataSubsystem* SaveDataSubsystem = GetGameInstance()->GetSubsystem<USaveDataSubsystem>();
	if (SaveDataSubsystem == nullptr)
	{
		LOGV(Error, TEXT("SaveDataSubsystem is not valid"));
		return;
	}

	FString SaveFileName = SaveFimeNameInputField->GetText().ToString();

	switch (CurrentConfirmationStep)
	{
	case EConfirmationStep::Save:

		ActivateWaitForSaveWidget();
		SaveDataSubsystem->AsyncSaveCurrentGame(SaveFileName, true);
		InitWidget();
		break;
	case EConfirmationStep::Load:

		if (SessionInfoList.IsValidIndex(CachedWidgetIndex) == false)
		{
			LOGV(Error, TEXT("Invalid Widget Index : %d"), CachedWidgetIndex);
			return;
		}

		SaveDataSubsystem->AsyncLoadSavedGame(SessionInfoList[CachedWidgetIndex].Key);
		break;
	case EConfirmationStep::Delete:

		if (SessionInfoList.IsValidIndex(CachedWidgetIndex) == false)
		{
			LOGV(Error, TEXT("Invalid Widget Index : %d"), CachedWidgetIndex);
			return;
		}

		SaveDataSubsystem->DeleteSavedGame(SessionInfoList[CachedWidgetIndex].Key);
		InitWidget();
		break;
	case EConfirmationStep::MAX:

		if (SessionInfoList.IsValidIndex(CachedWidgetIndex) == false)
		{
			LOGV(Error, TEXT("Invalid Confirmation Step"));
			return;
		}
		return;
	default:
		check(false);
		return;;
	}

	ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Hidden);
}

void USessionSaveLoadWidget::OnNoButtonClickedWithConfirmationDialog()
{
	CurrentConfirmationStep = EConfirmationStep::MAX;
	ConfirmationDialogWidget->SetVisibility(ESlateVisibility::Hidden);
}

void USessionSaveLoadWidget::ActivateWaitForSaveWidget()
{
	WaitForSaveAndLoadWidget->SetWaitText(TEXT("Wait for Save..."));
	WaitForSaveAndLoadWidget->SetVisibility(ESlateVisibility::Visible);
}

void USessionSaveLoadWidget::ActivateWaitForLoadWidget()
{
	WaitForSaveAndLoadWidget->SetWaitText(TEXT("Wait for Load..."));
	WaitForSaveAndLoadWidget->SetVisibility(ESlateVisibility::Visible);
}

void USessionSaveLoadWidget::SetSessionInfoToClickedColor(int32 WidgetIndex)
{
	UObject* ItemFromWidgetIndex = SaveInfoListView->GetItemAt(WidgetIndex);
	USavedSessionInfoWidget* WidgetEntryFromPreviousItem = Cast<USavedSessionInfoWidget>(SaveInfoListView->GetEntryWidgetFromItem(ItemFromWidgetIndex));
	if (WidgetEntryFromPreviousItem)
	{
		WidgetEntryFromPreviousItem->SetToClickedColor();
	}
}

void USessionSaveLoadWidget::SetSessionInfoToNotClickedColor(int32 WidgetIndex)
{
	UObject* ItemFromWidgetIndex = SaveInfoListView->GetItemAt(WidgetIndex);
	USavedSessionInfoWidget* WidgetEntryFromPreviousItem = Cast<USavedSessionInfoWidget>(SaveInfoListView->GetEntryWidgetFromItem(ItemFromWidgetIndex));
	if (WidgetEntryFromPreviousItem)
	{
		WidgetEntryFromPreviousItem->SetToNotClickedColor();
	}
}
