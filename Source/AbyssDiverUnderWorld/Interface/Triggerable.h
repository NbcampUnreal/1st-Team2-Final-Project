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
	void TriggerEventBeginOverlap(AActor* Instigator, AActor* ActingEventTrigger);
	virtual void TriggerEventBeginOverlap_Implementation(AActor* Instigator, AActor* ActingEventTrigger) = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void TriggerEventEndOverlap(AActor* Instigator, AActor* ActingEventTrigger);
	virtual void TriggerEventEndOverlap_Implementation(AActor* Instigator, AActor* ActingEventTrigger) = 0;
};
