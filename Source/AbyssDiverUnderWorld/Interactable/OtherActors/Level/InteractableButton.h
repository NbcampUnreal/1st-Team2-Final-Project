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
public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION()
	void OnRep_SetMesh();

	void SetStaticMesh(UStaticMesh* Mesh);


	FOnButtonPressed OnButtonPressed;
#pragma endregion

#pragma region Variables
protected:
	UPROPERTY(Replicated, VisibleAnywhere)
	EButtonAction ButtonAction = EButtonAction::MAX;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(/*ReplicatedUsing = OnRep_SetMesh*/)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(ReplicatedUsing = OnRep_SetMesh)
	TObjectPtr<UStaticMesh> ReplicatedMesh; //추후 MeshComponent Replicate 설정 동기화 확인해보기

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

	FString ButtonDescription = TEXT("호스트만 조작 가능합니다.");

#pragma endregion

#pragma region Getters / Setters

public:
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual FString GetInteractionDescription() const override;
	virtual bool IsHoldMode() const override;
	EButtonAction GetButtonAction() const { return ButtonAction; }
	void SetButtonAction(EButtonAction ButtonType) { ButtonAction = ButtonType; }
	void SetButtonDescription(const FString& Description);
#pragma endregion

};
