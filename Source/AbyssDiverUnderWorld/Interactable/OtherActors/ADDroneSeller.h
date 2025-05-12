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

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return bIsActive; }
	UFUNCTION()
	void DisableSelling();
	UFUNCTION()
	void OnRep_IsActive();
	UFUNCTION()
	void OnRep_CurrentMoney();

protected:
	int32 SellAllExchangeableItems(AActor* InstigatorActor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
#pragma endregion

#pragma region Variable
public:
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = true;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMoney)
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
	virtual UADInteractableComponent* GetInteractableComponent() const override;
#pragma endregion

};
