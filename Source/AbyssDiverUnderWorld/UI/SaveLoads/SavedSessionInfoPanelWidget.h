#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "SavedSessionInfoPanelWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USavedSessionInfoPanelWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;

#pragma region Methods

public:

	void SetSavedNameText(const FString& NewText);
	void SetSaveDateText(const FString& NewText);
	void SetClearCountNumberText(int32 NewClearCountNum);

	// 배열에 담은 순서대로 표시, 4개 초과시 표시 하지 않음. 4개 미만이면 부족한 만큼 표시하지 않음
	void SetPlayerNameTexts(const TArray<FString>& PlayerNameTexts);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SaveNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SaveDateText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ClearCountNumberText;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> PlayerNameTextArray;

private:

	static const int32 MaxPlayerNumber;

#pragma endregion
};
