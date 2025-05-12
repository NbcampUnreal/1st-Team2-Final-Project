#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Create;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Options;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Credits;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CreateTeamWidgetClass;

	UPROPERTY()
	UUserWidget* CreateTeamWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SessionListWidgetClass;

	UPROPERTY()
	UUserWidget* SessionListWidgetInstance;

};
