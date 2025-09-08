#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveData/CoopData/SavedSessionInfoSaveGame.h"
#include "SessionSaveLoadWidget.generated.h"

class UButton;
class UEditableTextBox;
class UListView;
class USavedSessionInfoPanelWidget;
class USavedSessionInfoWidgetData;
class USavedSessionInfoWidget;
class UWaitForSomethingWidget;
class UConfirmationDialogWidget;

//struct FSavedSessionInfo;

enum class EConfirmationStep
{
	Save,
	Load,
	Delete,
	MAX
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USessionSaveLoadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

#pragma region Methods

public:

	void InitWidget();

private:

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnSaveButtonClicked();

	UFUNCTION()
	void OnLoadButtonClicked();

	UFUNCTION()
	void OnDeleteButtonClicked();

	void OnInfoWidgetUpdated(USavedSessionInfoWidget* UpdatedWidget);

	void OnInfoWidgetClicked(int32 ClickedWidgetIndex);

	void OnSaveCompleted();

	void OnYesButtonClickedWithConfirmationDialog();
	void OnNoButtonClickedWithConfirmationDialog();

	void ActivateWaitForSaveWidget();
	void ActivateWaitForLoadWidget();

	void SetSessionInfoToClickedColor(int32 WidgetIndex);
	void SetSessionInfoToNotClickedColor(int32 WidgetIndex);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SaveFimeNameInputField;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> SaveInfoListView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USavedSessionInfoPanelWidget> SavedSessionInfoPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWaitForSomethingWidget> WaitForSaveAndLoadWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UConfirmationDialogWidget> ConfirmationDialogWidget;

	TArray<TPair<FString, FSavedSessionInfo>> SessionInfoList;

	EConfirmationStep CurrentConfirmationStep = EConfirmationStep::MAX;

	int32 CachedWidgetIndex = INDEX_NONE;

#pragma endregion	
};
