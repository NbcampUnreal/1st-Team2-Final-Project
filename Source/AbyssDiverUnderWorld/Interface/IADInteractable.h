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
	UFUNCTION()
	virtual void Interact(AActor* InstigatorActor) = 0;

	virtual UADInteractableComponent* GetInteractableComponent() const = 0;
};
