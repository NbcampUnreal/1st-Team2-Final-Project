#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Tutorial/TutorialEnums.h"
#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"
#include "ADTutorialGameMode.generated.h"

class UDataTable;
class ALight;
class APostProcessVolume;
class AUnderwaterCharacter;
class UAnimMontage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameMode : public AGameMode
{
	GENERATED_BODY()

	public:
	AADTutorialGameMode();

protected:
	virtual void StartPlay() override;

public:
#pragma region Method
public:
	void StartFirstTutorialPhase();
	void AdvanceTutorialPhase();
	void OnTypingAnimationFinished();
	void OnPlayerItemAction(EPlayerActionTrigger ItemActionType);
	void OnPlayerAttemptedRevive();
	void DestroyActiveWall();
	void SpawnNewWall(FName WallTag);

	UFUNCTION()
	void OnTrackedOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION(BlueprintCallable)
	void SpawnDownedNPC();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnPhaseBatteryStart();

	void HandleCurrentPhase();
	void HidePhaseActors();
	void OnReviveAnimationFinished();

	void HandlePhase_Movement();
	void HandlePhase_Sprint();
	void HandlePhase_Oxygen();
	void HandlePhase_Radar();
	void HandlePhase_Dialogue_02();
	void HandlePhase_Looting();
	void HandlePhase_Inventory();
	void HandlePhase_Drone();
	void HandlePhase_LightToggle();
	void HandlePhase_Items();
	void HandlePhase_Dialogue_06();
	void HandlePhase_Battery();
	void HandlePhase_Drop();
	void HandlePhase_Dialogue_05();
	void HandlePhase_OxygenWarning();
	void HandlePhase_Revive();
	void HandlePhase_Die();
	void HandlePhase_Resurrection();
	void HandlePhase_Complete();

private:
	void TrackPhaseActor(AActor* Actor);
	void BindIndicatorToOwner(AActor* OwnerActor, AActor* IndicatorActor);
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Battery")
	float BatteryStartPercentOverride = -1.f;

protected:
	UPROPERTY(EditAnywhere, Category = "Tutorial|Debug")
	ETutorialPhase StartPhaseOverride = ETutorialPhase::None;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Data")
	TObjectPtr<UDataTable> TutorialDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AActor> LootableOreClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AIndicatingTarget> IndicatingTargetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AUnderwaterCharacter> GroggyNPCClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Spawning")
	TSubclassOf<AActor> CurrentWallClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Spawning")
	FName OreSpawnTag;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Spawning")
	FName DialogueTargetSpawnTag;

	UPROPERTY(EditAnywhere, Category = "Tutorial|Spawning")
	FName GroggyNPCSpawnTag;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> LootingOreIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> DialogueIndicatorIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Settings | Icons")
	TObjectPtr<UTexture2D> DroneIndicatorIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Animation")
	TObjectPtr<UAnimMontage> TutorialReviveMontage;

	UPROPERTY()
	TWeakObjectPtr<AUnderwaterCharacter> TutorialNPC;

	FTimerHandle TutorialStartTimerHandle;
	FTimerHandle GroggyAnimationTimerHandle;

	uint8 bBatteryGaugeStarted : 1;

private:
	UPROPERTY()
	TObjectPtr<class AADPlayerController> TutorialPlayerController;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsToShowThisPhase;

	UPROPERTY()
	TArray<TObjectPtr<ALight>> DisabledLights;

	UPROPERTY()
	TObjectPtr<AActor> ActiveCurrentWall;

	UPROPERTY()
	TObjectPtr<APostProcessVolume> TutorialPPV = nullptr;

	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> OwnerToIndicator;

	bool bIsTypingFinishedForCurrentPhase = false;
	int32 ItemsPhaseProgress;
#pragma endregion

#pragma region Getter, Setter
public:
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTypingFinishedForCurrentPhase() const;
#pragma endregion
};