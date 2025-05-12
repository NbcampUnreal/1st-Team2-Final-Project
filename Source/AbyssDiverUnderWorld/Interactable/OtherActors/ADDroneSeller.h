#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "ADDroneSeller.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADDroneSeller : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADDroneSeller();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(BlueprintNativeEvent)
	void Interact(AActor* InstigatorActor);
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanHighlight() const;
	virtual bool CanHighlight_Implementation() const override { return true; }
	UFUNCTION()
	void DisableSelling();

protected:
	int32 SellAllExchangeableItems(AActor* InstigatorActor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
#pragma endregion

#pragma region Variable
public:
	
protected:
	UPROPERTY(Replicated)
	bool bIsActive = true;
	UPROPERTY(Replicated)
	int32 CurrentMoney = 0;
	UPROPERTY(EditAnywhere)
	int32 TargetMoney = 1000;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class AADDrone> LinkedDrone = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UADInteractableComponent> InteractableComp;
	

private:

#pragma endregion

#pragma region Getter, Setteer
public:
	int32 GetCurrentMoney() const { return CurrentMoney; }
	int32 GetTargetMoney() const { return TargetMoney; }

#pragma endregion

};
