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
	
public:
	void PopulateSessionList(const TArray<FOnlineSessionSearchResult>& SessionResults);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void OnBackClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* Scroll_SessionList;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Refresh;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Back;

	UPROPERTY(editDefaultsOnly, Category = "Session")
	TSubclassOf<USessionEntryWidget> SessionEntryWidgetClass;
};
