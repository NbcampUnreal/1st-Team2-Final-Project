#pragma once

#include "CoreMinimal.h"
#include "Framework/ADPlayerController.h"
#include "InputAction.h"
#include "ADTutorialPlayerController.generated.h"

class ATutorialManager;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialPlayerController : public AADPlayerController
{
	GENERATED_BODY()

public:
	AADTutorialPlayerController();

	virtual void BeginPlay() override;

	void RequestAdvanceTutorialPhase();

	void ReportItemAction(EPlayerActionTrigger ItemActionType);

protected:

	virtual void SetPawn(APawn* InPawn) override;

	void OnInteractStarted(const FInputActionValue& Value);
	void OnInteractCompleted(const FInputActionValue& Value);

	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);

	void OnRadarStarted(const FInputActionValue& Value);
	void OnRadarCompleted(const FInputActionValue& Value);

	void OnInventoryStarted(const FInputActionValue& Value);
	void OnInventoryCompleted(const FInputActionValue& Value);

	void OnLightToggleStarted(const FInputActionValue& Value);
	void OnLightToggleCompleted(const FInputActionValue& Value);

	void OnBatteryStarted(const FInputActionValue& Value);
	void OnBatteryCompleted(const FInputActionValue& Value);

	void OnDropStarted(const FInputActionValue& Value);
	void OnDropCompleted(const FInputActionValue& Value);

	void OnReviveStarted(const FInputActionValue& Value);
	void OnReviveCompleted(const FInputActionValue& Value);

	void CheckTutorialObjective(const FInputActionValue& Value, UInputAction* SourceAction);

private:
	UPROPERTY()
	TObjectPtr<ATutorialManager> CachedTutorialManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ReviveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> BatteryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DropAction;
};