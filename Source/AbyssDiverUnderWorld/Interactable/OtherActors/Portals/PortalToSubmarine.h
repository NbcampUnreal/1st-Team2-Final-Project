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
	

public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
};
