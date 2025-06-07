#include "Interactable/OtherActors/Portals/PortalToSubmarine.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerState.h"
#include "Subsystems/SoundSubsystem.h"

#include "EngineUtils.h"

void APortalToSubmarine::BeginPlay()
{
	Super::BeginPlay();

	GetSoundSubsystem()->PlayAt(ESFX::Submarine, GetActorLocation(), 2.0f);
}

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
	
	if (bIsNetCullingDeactivated)
	{
		return;
	}

	bIsNetCullingDeactivated = true;

	for (AADPlayerState* ADPlayerState : TActorRange<AADPlayerState>(GetWorld()))
	{
		ADPlayerState->GetPawn()->bAlwaysRelevant = true;
	}

	ForceNetUpdate();
	LOGV(Error, TEXT("Releveant On"));
}
