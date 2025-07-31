#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Blueprint/IUserObjectListEntry.h"

#include "SavedSessionInfoWidget.generated.h"

class UTextBlock;
class UImage;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInfoWidgetClickedDelegate, int32 /*WidgetIndex*/);

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USavedSessionInfoWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

#pragma region Methods

public:

	void SetBackgroundColor(const FLinearColor& NewColor);
	void SetSavedSessionNameText(const FString& NewText);
	void SetSavedSessionDateText(const FString& NewText);

	void SetToClickedColor();
	void SetToNotClickedColor();

	FOnInfoWidgetClickedDelegate OnInfoWidgetClickedDelegate;

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SavedSessionNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SavedSessionDateText;

	int32 WidgetIndex;

	static const FLinearColor ClickedColor;
	static const FLinearColor NotClickedColor;

#pragma endregion

#pragma region Getter / Setter

public:

	int32 GetWidgetIndex() const;

#pragma endregion


	
};
