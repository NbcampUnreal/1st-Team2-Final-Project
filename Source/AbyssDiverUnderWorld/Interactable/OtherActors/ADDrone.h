#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 
#include "Interface/IADInteractable.h"
#include "ADDrone.generated.h"

#define LOGD(Verbosity, Format, ...) UE_LOG(DroneLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(DroneLog, Log, All);

class UADInteractableComponent;
class AADDroneSeller;
class ASpawnManager;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADDrone : public AActor,  public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADDrone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region Method
public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	virtual bool CanHighlight_Implementation() const override { return bIsActive; }
	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayDroneRisingSound();
	void M_PlayDroneRisingSound_Implementation();
	
	UFUNCTION()
	void Activate();
	UFUNCTION()
	void OnRep_IsActive();
	void StartRising();
	void OnDestroyTimer();

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
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, EditAnywhere, BlueprintReadWrite)
	uint8 bIsActive : 1;
	UPROPERTY(EditAnywhere)
	TObjectPtr<AADDroneSeller> CurrentSeller = nullptr;
	UPROPERTY(EditAnywhere)
	TObjectPtr<AADDroneSeller> NextSeller = nullptr;
	UPROPERTY(EditAnywhere)
	TObjectPtr<ASpawnManager> SpawnManager = nullptr;
	UPROPERTY(EditAnywhere)
	float RaiseSpeed = 200.f;
	UPROPERTY(EditAnywhere)
	float DestroyDelay = 5.f;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

protected:

	// 드론에 해당하는 Phase를 나타내는 숫자
	UPROPERTY(EditAnywhere)
	int32 DronePhaseNumber = 0;

private:
	FTimerHandle DestroyHandle;

#pragma endregion

#pragma region Getter, Setteer
public:
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;

	int32 GetDronePhaseNumber() const { return DronePhaseNumber; }
	virtual FString GetInteractionDescription() const override;

private:
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion

	

};
