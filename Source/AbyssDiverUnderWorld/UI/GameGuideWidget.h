// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameGuideWidget.generated.h"

class UListView;
class UImage;
class URichTextBlock;
class UHorizontalBox;
struct FGameGuideContent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UGameGuideWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void PlayGameGuideAnimation(bool bIsInAnim);

	void ToggleGuideVisibility();

#pragma region Method
private:
	UFUNCTION(BlueprintCallable)
	void MovePage(bool bIsNext);

	UFUNCTION(BlueprintCallable)
	void SetGuideVisibility(bool bIsVisible);

	UFUNCTION()
	void OnSlotEntryWidgetUpdated(class UGameGuideListSlot* SlotEntryWidget);

	UFUNCTION()
	void UpdateGuideContentArray(int GuideId);

	void UpdateGuideList();

	void UpdateGuideContent(int ContentIdx);

	void SetVisibilityMovePageButton(bool bIsVisible);

#pragma endregion

#pragma region Valiable
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsAnimationPlaying : 1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> GuideList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> GuideImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> GuideDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> MovePageButton;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> ShowAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> HideAnim;
	 
	TArray<FGameGuideContent> Contents;

	int8 ContentsIdx = 0;

	uint8 bIsVisibility : 1 = false;
#pragma endregion

#pragma region Getter/Setter
public:
	UFUNCTION(BlueprintCallable)
	void SetbIsAnimationPlaying(bool NewbIsAnimationPlaying);
	UFUNCTION(BlueprintCallable)
	bool GetbIsAnimationPlaying() { return bIsAnimationPlaying; };
	UFUNCTION(BlueprintCallable)
	bool GetbIsVisibility() { return bIsVisibility; };
#pragma endregion
};
