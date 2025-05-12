#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionListWidget.generated.h"

class UScrollBox;
class UButton;
class USessionEntryWidget;


UCLASS()
class ABYSSDIVERUNDERWORLD_API USessionListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBackClicked();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> Scroll_SessionList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

};
