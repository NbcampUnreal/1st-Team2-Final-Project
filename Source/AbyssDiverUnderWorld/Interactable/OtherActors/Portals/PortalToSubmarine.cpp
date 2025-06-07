#include "Interactable/OtherActors/Portals/PortalToSubmarine.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerState.h"

void APortalToSubmarine::Interact_Implementation(AActor* InstigatorActor)
{
	Super::Interact_Implementation(InstigatorActor);

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(InstigatorActor);
	if (PlayerCharacter == nullptr)
	{
		LOGV(Warning, TEXT("PlayerCharacter == nullptr"));
		return;
	}

	AADPlayerState* PS = Cast<AADPlayerState>(PlayerCharacter->GetPlayerState());
	if (PS == nullptr)
	{
		LOGV(Warning, TEXT("PS == nullptr"));
		return;
	}

	PS->SetIsSafeReturn(true);
}
