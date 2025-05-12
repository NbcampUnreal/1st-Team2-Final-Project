#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Container/FStructContainer.h"
#include "ADItemBase.generated.h"

class UADInteractableComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADItemBase : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	AADItemBase();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:	

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	virtual void HandlePickup(APawn* InstigatorPawn);

protected:
	UFUNCTION()
	void OnRep_ItemData();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(ReplicatedUsing = OnRep_ItemData, EditAnywhere, Category = "Item")
	FItemData ItemData;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UADInteractableComponent> InteractableComp;

	// TODO : 인벤토리 컴포넌트 참조
	// TODO : PickupSound 등 획득 시 효과 추가
protected:
	

private:

#pragma endregion

#pragma region Getter, Setteer
public:
	void SetItemMass(int32 InMass);
	void SetPrice(int32 InPrice);
	virtual UADInteractableComponent* GetInteractableComponent() const override;
#pragma endregion

};
