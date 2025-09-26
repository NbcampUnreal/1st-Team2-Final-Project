#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateTeamWidget.h"
#include "SessionListWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UCreateTeamWidget;
class USessionListWidget;
class UOptionsWidget;
class UCreditsWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION()
	void OnCreateClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnTutorialClicked();

	UFUNCTION()
	void OnOptionsClicked();

	UFUNCTION()
	void OnCreditsClicked();

	UFUNCTION()
	void OnQuitClicked();

	void OnCreateTeamWidgetClosed();

	void OnSessionListWidgetClosed();

	void OnOptionsWidgetClosed();

	void OnCreditsWidgetClosed();


protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Create;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Join;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Tutorial;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Credits;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCreateTeamWidget> CreateTeamWidgetClass;

	UPROPERTY()
	TObjectPtr<UCreateTeamWidget> CreateTeamWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USessionListWidget> SessionListWidgetClass;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USessionListWidget> SessionListWidgetInstance;

	/*UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UOptionsWidget> OptionsWidgetClass;

	UPROPERTY()
	TObjectPtr<UOptionsWidget> OptionsWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCreditsWidget> CreditsWidgetClass;

	UPROPERTY()
	TObjectPtr<UCreditsWidget> CreditsWidgetInstance;
	*/

};
