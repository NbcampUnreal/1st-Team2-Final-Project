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

	FOnPhaseChangeDelegate OnPhaseChangeDelegate;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UADInteractableComponent> InteractableComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Money")
	int32 AccumulatedMoney = 0;
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, EditAnywhere, BlueprintReadWrite, Category = "DroneSettings")
	uint8 bIsActive : 1;
	UPROPERTY(Replicated)
	uint8 bIsFlying : 1;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<AADDroneSeller> CurrentSeller = nullptr;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<AADDroneSeller> NextSeller = nullptr;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	TObjectPtr<ASpawnManager> SpawnManager = nullptr;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float RaiseSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float DestroyDelay = 5.f;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;
	UPROPERTY()
	TObjectPtr<UADWorldSubsystem> WorldSubsystem;
	UPROPERTY(EditAnywhere)
	UDataTable* PhaseBgmTable;

protected:

	// 드론에 해당하는 Phase를 나타내는 숫자
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	int32 DronePhaseNumber = 0;

	/** 사망 부활 반경 */
	UPROPERTY(EditAnywhere, Category = "DroneSettings")
	float ReviveDistance;

	UPROPERTY(EditInstanceOnly, Category = "DroneSettings")
	TArray<TObjectPtr<ATargetPoint>> PlayerRespawnLocations;

private:

	FTimerHandle DestroyHandle;
	int32 CachedSoundNumber;
	int32 DrondeThemeSoundNumber;

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
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion

	

};
