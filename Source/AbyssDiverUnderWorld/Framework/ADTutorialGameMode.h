#pragma once

#include "CoreMinimal.h"
#include "Framework/ADInGameMode.h"
#include "Tutorial/TutorialEnums.h"
#include "Sound/SoundMix.h"
#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"
#include "Character/PlayerComponent/OxygenComponent.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/PlayerController.h"
#include "ADTutorialGameMode.generated.h"

class UDataTable;
class ALight;
class APostProcessVolume;
class AUnderwaterCharacter;
class UAnimMontage;
class AADDroneSeller;
class AADDrone;
class AADPlayerController;
class UTexture2D;
class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameMode : public AADInGameMode
{
    GENERATED_BODY()

public: 
    AADTutorialGameMode();

protected: 
    virtual void StartPlay() override;

#pragma region Method
public:
    UFUNCTION(Server, Reliable)
    void NotifyBodySubmitted(AUnderwaterCharacter* SubmittingPlayer);

    // 튜토리얼 흐름
    void StartFirstTutorialPhase();
    void AdvanceTutorialPhase();
    void OnTypingAnimationFinished();
    void OnPlayerItemAction(EPlayerActionTrigger ItemActionType);

    // 벽
    void DestroyActiveWall();
    void SpawnNewWall(FName WallTag);

    // 기타
    void TriggerResurrectionSequence();

    // 접근자
    UFUNCTION(BlueprintPure, Category = "Tutorial")
    bool IsTypingFinishedForCurrentPhase() const;

    AUnderwaterCharacter* GetTutorialNPC() const;

    // 리스폰/부활
    void ReviveSinglePlayerAtDrone(int8 PlayerIndex, const AADDrone* Drone);
    bool TutorialTryGetRandomTaggedTargetPointTransform(const FName& Tag, FTransform& OutTM) const;
    FTransform GetBestTutorialRespawnTransform(const class AADDrone* Drone) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void RequestFinishTutorial();
protected:
    // Phase 핸들러
    void HandleCurrentPhase();
    void HandlePhase_Movement();
    void HandlePhase_Sprint();
    void HandlePhase_Oxygen();
    void HandlePhase_Radar();
    void HandlePhase_Dialogue_02();
    void HandlePhase_Looting();
    void HandlePhase_Inventory();
    void HandlePhase_Drone();
    void HandlePhase_Dialogue_LightOut();
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

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void OnPhaseBatteryStart();

    UFUNCTION()
    void OnTutorialNPCStateChanged(AUnderwaterCharacter* Character,
        ECharacterState OldCharacterState,
        ECharacterState NewCharacterState);

    // 리스폰 보조
    class AADTutorialPlayerController* FindTutorialPlayerControllerByIndex(int8 PlayerIndex) const;
    void RestartTutorialPlayerFromIndex(int8 PlayerIndex, const FVector& SpawnLocation);
    FVector GetTutorialRandomLocation(const FVector& Center, float Distance) const;

private:
    void HidePhaseActors();
    void TrackPhaseActor(AActor* Actor);
    void BindIndicatorToOwner(AActor* OwnerActor, AActor* IndicatorActor);

    UFUNCTION()
    void OnTrackedOwnerDestroyed(AActor* DestroyedActor);

    void SpawnTutorialDronePair(TObjectPtr<AADDroneSeller>& OutSeller, TObjectPtr<AADDrone>& OutDrone);
    FTransform GetResurrectionSpawnTransform() const;

    void ReviveTutorialNPCAtDrone(AADDrone* Drone);

    ATargetPoint* FindTargetPointByTag(const FName& Tag) const;

    FTransform ChooseTutorialRespawnTransform(const AADDrone* Drone) const;
#pragma endregion

#pragma region Variable
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Battery")
    float BatteryStartPercentOverride = -1.f;

    UPROPERTY()
    TObjectPtr<USoundSubsystem> TutorialSoundSubsystem;

protected:
    UPROPERTY(EditAnywhere, Category = "Tutorial|Debug")
    ETutorialPhase StartPhaseOverride = ETutorialPhase::None;

    // Spawning
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

    // Icons
    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Settings|Icons")
    TObjectPtr<UTexture2D> LootingOreIcon;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Settings|Icons")
    TObjectPtr<UTexture2D> DialogueIndicatorIcon;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Settings|Icons")
    TObjectPtr<UTexture2D> DroneIndicatorIcon;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Settings|Icons")
    TObjectPtr<UTexture2D> NPCIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial|Indicators")
    TObjectPtr<UTexture2D> LadderExitIndicatorIcon;

    // Timing
    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Timing")
    float EmoteToNextPhaseDelay = 2.0f;

    // Sound
    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Sound")
    TObjectPtr<USoundMix> MuteDroneSoundMix;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Sound")
    TObjectPtr<USoundBase> LightOutSound;

    // Drone
    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Drone")
    TSubclassOf<AADDroneSeller> TutorialDroneSellerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Drone")
    TSubclassOf<AADDrone> TutorialDroneClass;

    UPROPERTY(EditAnywhere, Category = "Tutorial|Drone")
    FName ResurrectionDroneSpawnTag;

    // Respawn
    UPROPERTY(EditAnywhere, Category = "Tutorial|Respawn")
    FName FixedRespawnPointTag = FName("RespawnPoint");
private:
    UPROPERTY()
    TWeakObjectPtr<AUnderwaterCharacter> TutorialNPC;

    // 컨트롤러/액터 캐시
    UPROPERTY()
    TObjectPtr<AADPlayerController> TutorialPlayerController;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> ActorsToShowThisPhase;

    UPROPERTY()
    TArray<TObjectPtr<ALight>> DisabledLights;

    UPROPERTY()
    TObjectPtr<AActor> ActiveCurrentWall;

    UPROPERTY()
    TObjectPtr<APostProcessVolume> TutorialPPV;

    // 드론/셀러
    UPROPERTY()
    TObjectPtr<AADDroneSeller> Tutorial_ActiveSeller;

    UPROPERTY()
    TObjectPtr<AADDrone> Tutorial_ActiveDrone;

    TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> OwnerToIndicator;

    // 타이머
    FTimerHandle TutorialStartTimerHandle;
    FTimerHandle EmoteToNextTimerHandle;

    // 플래그/상태
    uint8 bBatteryGaugeStarted : 1;
    uint8 bIsTypingFinishedForCurrentPhase : 1;
    uint8 bIsBodySubmittedInResurrectionPhase : 1;

    int32 ItemsPhaseProgress = 0;

    int8 DroneTutorialAlarmId;
#pragma endregion
};
