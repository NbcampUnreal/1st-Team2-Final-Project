#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Tutorial/TutorialEnums.h"
#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"
#include "ADTutorialGameMode.generated.h"

class UDataTable;
class ALight;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AADTutorialGameMode();

	virtual void StartPlay() override;
	void AdvanceTutorialPhase();
	void OnTypingAnimationFinished();
	void OnPlayerItemAction(EPlayerActionTrigger ItemActionType);

	UFUNCTION()
	void OnTrackedOwnerDestroyed(AActor* DestroyedActor);

protected:
	void HandleCurrentPhase();
	void HandlePhase_Dialogue_02();
	void HandlePhase_Movement();
	void HandlePhase_Sprint();
	void HandlePhase_Oxygen();
	void HandlePhase_Radar();
	void HandlePhase_Looting();
	void HandlePhase_Inventory();
	void HandlePhase_Drone();
	void HandlePhase_LightToggle();
	void HandlePhase_Items();
	void HandlePhase_Battery();
	void HandlePhase_Drop();
	void HandlePhase_OxygenWarning();
	void HandlePhase_Revive();
	void HandlePhase_Die();
	void HandlePhase_Resurrection();
	void HandlePhase_Complete();

	UFUNCTION(BlueprintCallable)
	void SpawnDownedNPC();

	void HidePhaseActors();

private:
	void TrackPhaseActor(AActor* Actor) { if (IsValid(Actor)) { ActorsToShowThisPhase.Add(Actor); } }
	void BindIndicatorToOwner(AActor* OwnerActor, AActor* IndicatorActor);

	UPROPERTY()
	TObjectPtr<class AADPlayerController> TutorialPlayerController;

	FTimerHandle StepTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> TutorialDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AActor> LootableOreClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AIndicatingTarget> IndicatingTargetClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Spawning")
	FName OreSpawnTag;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Spawning")
	FName DialogueTargetSpawnTag;

	bool bIsTypingFinishedForCurrentPhase = false;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsToShowThisPhase;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> LootingOreIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> DialogueIndicatorIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> DroneIndicatorIcon;

	UPROPERTY()
	TArray<TObjectPtr<ALight>> DisabledLights;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> OwnerToIndicator;

	int32 ItemsPhaseProgress;
};