#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MissionEntryOnHUDWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;


UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionEntryOnHUDWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Methods

public:
	void ChangeImage(UTexture2D* Image);
	void UpdateMissionEntryColor(bool bIsMissionCompleted);
	void SetVisible(bool bShouldVisible);

	UFUNCTION(BlueprintCallable) 
	void SetTitle(const FText& InTitle);
	UFUNCTION(BlueprintCallable) 
	void SetDescription(const FText& InDesc);
	UFUNCTION(BlueprintCallable) 
	void SetProgress(int32 Current, int32 Goal);
	UFUNCTION(BlueprintCallable) 
	void SetCompleted(bool bCompleted); // 회색 처리 등


#pragma endregion

#pragma region Variables

protected:


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MissionEntryImage;

	// 새로 추가: 텍스트/프로그레스 (UMG에서 BindWidget로 연결)
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Title;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Description;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Progress;

	static const FLinearColor CompleteColor;
	static const FLinearColor IncompleteColor;


#pragma endregion
	
};
