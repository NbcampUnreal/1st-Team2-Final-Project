#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/IADInteractable.h"

#include "LevelTraveler.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ALevelTraveler : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	

	ALevelTraveler();

protected:

	virtual void BeginPlay() override;

#pragma region Methods

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	bool IsConditionMet();
#pragma endregion

#pragma region Variables

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditAnywhere, Category = "LevelTraveler")
	uint8 bIsHoldMode : 1;

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

#pragma endregion



#pragma region Getters / Setters

public:

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;

#pragma endregion



};
