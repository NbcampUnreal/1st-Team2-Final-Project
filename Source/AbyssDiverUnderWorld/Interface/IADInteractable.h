#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "IADInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIADInteractable : public UInterface
{
	GENERATED_BODY()
};

class ABYSSDIVERUNDERWORLD_API IIADInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* InstigatorActor);
	virtual void Interact_Implementation(AActor* InstigatorActor);
	// Ȧ�� ���� �� ȣ���� �Լ�
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void InteractHold(AActor* InstigatorActor);
	virtual void InteractHold_Implementation(AActor* InstigatorActor);
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanHighlight() const;
	virtual bool CanHighlight_Implementation() const { return true; }
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	float GetHoldDuration() const;
	virtual float GetHoldDuration_Implementation() const;

	virtual UADInteractableComponent* GetInteractableComponent() const = 0;
	virtual bool IsHoldMode() const = 0;
};
