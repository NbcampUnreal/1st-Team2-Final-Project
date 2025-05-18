#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoldInteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UHoldInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
	// 홀드 시작 시 호출할 함수
	UFUNCTION()
	void HandleHoldStart(AActor* Target, float Duration);
	// 홀드 실패 시 호출할 함수
	UFUNCTION()
	void HandleHoldCancel();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HoldProgressBar;
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
