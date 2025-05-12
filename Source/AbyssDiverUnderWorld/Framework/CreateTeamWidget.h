#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateTeamWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UCreateTeamWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;


protected:
	UFUNCTION()
	void OnPublicClicked();

	UFUNCTION()
	void OnPrivateClicked();

	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnConfirmClicked();

	void UpdateSelection();
protected:
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* TextBox_ServerName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Public;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Private;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Back;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Confirm;

	uint8 bIsPrivate : 1 = 0;
};
