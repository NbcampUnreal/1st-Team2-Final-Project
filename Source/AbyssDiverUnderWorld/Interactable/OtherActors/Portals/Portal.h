#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/IADInteractable.h"

#include "Portal.generated.h"

class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API APortal : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	

	APortal();

protected:

	virtual void BeginPlay() override;

#pragma region Methods

protected:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	bool IsConditionMet();

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditInstanceOnly, Category = "Portal")
	TObjectPtr<class ATargetPoint> TaregetDestination;

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

	UPROPERTY(EditAnywhere, Category = "Portal")
	uint8 bIsHoldMode : 1;

	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;
	USoundSubsystem* GetSoundSubsystem();

#pragma endregion



};
