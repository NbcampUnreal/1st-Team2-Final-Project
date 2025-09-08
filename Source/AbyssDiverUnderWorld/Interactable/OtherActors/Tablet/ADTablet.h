#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "ADTablet.generated.h"

class UWidgetComponent;
class UCameraComponent;
class AUnderwaterCharacter;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTablet : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADTablet();

public:
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return !bIsHeld; }
	void Pickup(AUnderwaterCharacter* UnderwaterCharacter);
	UFUNCTION(BlueprintCallable)
	void PutDown();

protected:
	UFUNCTION()
	void OnRep_Held();
	//UFUNCTION()
	//void OnRep_HeldBy();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty>& OutLifetimeProps) const override;

private:
#pragma endregion

#pragma region Variable
public:


protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TabletMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UADInteractableComponent> InteractableComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> ScreenWidget;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Held)
	uint8 bIsHeld : 1;
	UPROPERTY(Replicated)
	TObjectPtr<AUnderwaterCharacter> HeldBy;
	UPROPERTY(EditAnywhere)
	FVector HoldOffsetLocation = FVector(50.f, 0.f, -20.f);
	UPROPERTY(EditAnywhere)
	FRotator HoldOffsetRotation = FRotator(-10.f, 0.f, 0.f);
	UCameraComponent* OwnerCamera = nullptr;
	FTransform CachedWorldTransform;

#pragma endregion

#pragma region Getter, Setteer
public:
	virtual UADInteractableComponent* GetInteractableComponent() const override { return InteractableComp; }
	virtual bool IsHoldMode() const override { return false; }
	virtual FString GetInteractionDescription() const override;

private:
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion


};
