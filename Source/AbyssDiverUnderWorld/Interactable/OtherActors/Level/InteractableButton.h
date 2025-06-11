// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "InteractableButton.generated.h"

UENUM(BlueprintType)
enum class EButtonAction : uint8
{
	PrevLevel = 0,
	NextLevel = 1,
	SelectLevel = 2,
	MAX = 3 UMETA(Hidden)
};

DECLARE_DELEGATE_OneParam(FOnButtonPressed, AActor*);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AInteractableButton : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	AInteractableButton();

#pragma region Methods

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void M_SetStaticMesh(UStaticMesh* Mesh);
	void M_SetStaticMesh_Implementation(UStaticMesh* Mesh);

	FOnButtonPressed OnButtonPressed;
#pragma endregion

#pragma region Variables
protected:
	UPROPERTY(Replicated)
	EButtonAction ButtonAction = EButtonAction::MAX;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

#pragma endregion

#pragma region Getters / Setters

public:
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	EButtonAction GetButtonAction() const { return ButtonAction; }
	void SetButtonAction(EButtonAction ButtonType) { ButtonAction = ButtonType; }
#pragma endregion

};
