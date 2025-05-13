#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateTeamWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

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
	void OnOptionsClicked();

	UFUNCTION()
	void OnCreditsClicked();

	UFUNCTION()
	void OnQuitClicked();

	void OnCreateTeamWidgetClosed();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Create;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Join;

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
	

};
