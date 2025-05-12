#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateTeamWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

DECLARE_DELEGATE(FOnBackClickedDelegate);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UCreateTeamWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FOnBackClickedDelegate OnBackClickedDelegate;
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
	TObjectPtr<UButton> Button_Public;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Private;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;


	UPROPERTY(BlueprintReadOnly)
	uint8 bIsPrivate : 1 = 0;
};
