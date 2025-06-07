#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/IADInteractable.h"
#include "InteractionDescriptionWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UInteractionDescriptionWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
#pragma region Method
public:
	// 상호작용 가능할 때 호출할 함수
	UFUNCTION()
	void HandleFocus(AActor* Actor, FString Description);
	// 상호작용 불가능할 때 호출할 함수
	UFUNCTION()
	void HandleFocusLost();

protected:


#pragma endregion

#pragma region Variable
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextAction;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageKeyE;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> InAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> OutAnim;

private:
	FTimerHandle TimerHandle;
	float Elapsed = 0.f;
	float TotalDuration = 0.f;
	uint8 bIsHolding : 1;

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

};
