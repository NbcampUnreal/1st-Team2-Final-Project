#pragma once

#include "CoreMinimal.h"
#include "Interactable/OtherActors/Portals/Portal.h"

#include "PortalToSubmarine.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API APortalToSubmarine : public APortal
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool IsConditionMet() override;

private:

	uint8 bIsNetCullingDeactivated : 1 = false;
};
