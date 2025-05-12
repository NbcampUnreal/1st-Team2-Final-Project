#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 
#include "Interface/IADInteractable.h"
#include "ADDrone.generated.h"

class UADInteractableComponent;

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
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* InstigatorActor);
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanHighlight() const;
	virtual bool CanHighlight_Implementation() const override { return true; }
	UFUNCTION()
	void Activate(class AADDroneSeller* Seller);
	void StartRising();
	void OnDestroyTimer();

protected:


private:
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UADInteractableComponent> InteractableComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Money")
	int32 TargetMoney = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Money")
	int32 AccumulatedMoney = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;
	UPROPERTY()
	AADDroneSeller* SellerRef = nullptr;
	UPROPERTY(EditAnywhere)
	float RaiseSpeed = 200.f;
	UPROPERTY(EditAnywhere)
	float DestroyDelay = 5.f;

protected:


private:
	FTimerHandle DestroyHandle;

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

	

};
