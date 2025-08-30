#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObserveOverlayWidget.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UObserveOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetObserveModeActive(bool bActive);
	void SetRingVisible(bool bVisible);
	void SetRingProgress(float Progress01);
	void SetRingScreenPos(const FVector2D& ScreenPos);
	void PlayAcquirePulse();
	void AttachObservedTargetWidget(class UObservedTargetWidget* InWidget);

protected:
	// 여기부터는 전부 WBP에서 구현하는 이벤트 (C++ 구현 불필요)
	UFUNCTION(BlueprintImplementableEvent) void BP_OnObserveModeActivated();
	UFUNCTION(BlueprintImplementableEvent) void BP_OnObserveModeDeactivated();
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetRingVisible(bool bVisible);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetRingProgress(float Progress01);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnSetRingScreenPosition(FVector2D ScreenPos);
	UFUNCTION(BlueprintImplementableEvent) void BP_OnAcquirePulse();
	UFUNCTION(BlueprintImplementableEvent) void BP_AttachObservedTarget(class UObservedTargetWidget* InWidget);
	
};
