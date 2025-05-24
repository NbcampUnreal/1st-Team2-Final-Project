#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Triggerable.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UTriggerable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ABYSSDIVERUNDERWORLD_API ITriggerable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent)
	void TriggerEvent(AActor* Instigator);
	virtual void TriggerEvent_Implementation(AActor* Instigator) = 0;
};
