#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ConfirmationDialogWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_MULTICAST_DELEGATE(FOnYesButtonClickedDelegate);
DECLARE_MULTICAST_DELEGATE(FOnNoButtonClickedDelegate);

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UConfirmationDialogWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

#pragma region Methods

public:

	void SetConfirmationText(const FString& NewText);

	FOnYesButtonClickedDelegate OnYesButtonClickedDelegate;
	FOnNoButtonClickedDelegate OnNoButtonClickedDelegate;

private:

	UFUNCTION()
	void OnYesButtonClicked();

	UFUNCTION()
	void OnNoButtonClicked();

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmationText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> YesButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NoButton;

#pragma endregion

	
};
