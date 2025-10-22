#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 
#include "Interface/IADInteractable.h"
#include "ADDrone.generated.h"

#define LOGD(Verbosity, Format, ...) UE_LOG(DroneLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(DroneLog, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhaseChangeDelegate, int32/*NextPhaseNumber*/);

class UADInteractableComponent;
class AADDroneSeller;
class ASpawnManager;
class USoundSubsystem;
class ATargetPoint;
class UADWorldSubsystem;
class UInteractPopupWidget;
enum class ESFX_BGM : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADDrone : public AActor,  public IIADInteractable
{
	GENERATED_BODY()
	
public:	

	AADDrone();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;

#pragma region Method
public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return bIsActive; }
	
	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayTutorialAlarmSound();
	void M_PlayTutorialAlarmSound_Implementation();
	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayDroneRisingSound();
	void M_PlayDroneRisingSound_Implementation();
	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayPhaseBGM(int32 PhaseNumber);
	void M_PlayPhaseBGM_Implementation(int32 PhaseNumber);
	
	UFUNCTION()
	void Activate();
	UFUNCTION()
	void OnRep_IsActive();
	void StartRising();
	void OnDestroyTimer();
	void ExecuteConfirmedInteraction();

	FOnPhaseChangeDelegate OnPhaseChangeDelegate;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Phase 번호에 해당하는 BGM 타입을 DataTable에서 가져옴 */
	ESFX_BGM GetPhaseBGM(int32 PhaseNumber) const;
	
private:
#pragma endregion

#pragma region Variable

public:
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<AADDroneSeller> CurrentSeller = nullptr;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UADInteractableComponent> InteractableComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive, EditAnywhere, BlueprintReadWrite, Category = "DroneSettings")
	uint8 bIsActive : 1;
	UPROPERTY(Replicated)
	uint8 bIsFlying : 1;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<AADDroneSeller> NextSeller = nullptr;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<ASpawnManager> SpawnManager = nullptr;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float RaiseSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float DestroyDelay = 5.f;
	UPROPERTY(EditAnywhere)
	UDataTable* PhaseBgmTable;

	// 드론에 해당하는 Phase를 나타내는 숫자
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	int32 DronePhaseNumber = 0;

	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	uint8 bIsBgmOn : 1;

	/** 사망 부활 반경 */
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float ReviveDistance;

	UPROPERTY(EditInstanceOnly, Category = "DroneSettings")
	TArray<TObjectPtr<ATargetPoint>> PlayerRespawnLocations;

	FTimerHandle DestroyHandle;

	/** 현재 재생 중인 BGM 오디오 ID */
	int32 BGMAudioID = INDEX_NONE;

	/** 드론 테마 오디오 ID */
	int32 DroneThemeAudioId = INDEX_NONE;

	/** 튜토리얼 알람 사운드 ID */
	int32 TutorialAlarmSoundId = INDEX_NONE;

private:
	
	UPROPERTY()
	mutable TWeakObjectPtr<USoundSubsystem> SoundSubsystemWeakPtr;
	
	UPROPERTY()
	mutable TWeakObjectPtr<UADWorldSubsystem> WorldSubsystemWeakPtr;
	
#pragma endregion

#pragma region Getter, Setteer
public:
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;

	int32 GetDronePhaseNumber() const { return DronePhaseNumber; }
	virtual FString GetInteractionDescription() const override;

	const TArray<ATargetPoint*>& GetPlayerRespawnLocations() const;
	float GetReviveDistance() const;

private:
	
	USoundSubsystem* GetSoundSubsystem() const;

	UADWorldSubsystem* GetWorldSubsystem() const;
	
#pragma endregion

	

};
