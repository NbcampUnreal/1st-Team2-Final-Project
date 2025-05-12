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
	UFUNCTION(BlueprintNativeEvent)
	void Interact(AActor* InstigatorActor);
	virtual void Interact_Implementation(AActor* InstigatorActor);
	// Ȧ�� ���� �� ȣ���� �Լ�
	UFUNCTION(BlueprintNativeEvent)
	void InteractHold(AActor* InstigatorActor);
	virtual void InteractHold_Implementation(AActor* InstigatorActor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanHighlight() const;
	virtual bool CanHighlight_Implementation() const { return true; }

	virtual UADInteractableComponent* GetInteractableComponent() const = 0;
};
