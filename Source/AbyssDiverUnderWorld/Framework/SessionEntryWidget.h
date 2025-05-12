#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "SessionEntryWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FOnlineSessionSearchResult& InResult);

protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnJoinClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SessionName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerCount;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;

	FOnlineSessionSearchResult SearchResult;
};
