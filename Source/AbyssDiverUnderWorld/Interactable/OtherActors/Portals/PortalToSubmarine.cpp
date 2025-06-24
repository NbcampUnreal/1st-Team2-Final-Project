#include "Interactable/OtherActors/Portals/PortalToSubmarine.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameMode.h"
#include "Subsystems/SoundSubsystem.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

void APortalToSubmarine::BeginPlay()
{
	Super::BeginPlay();

	GetSoundSubsystem()->PlayAt(ESFX::Submarine, GetActorLocation(), 2.0f);
}

void APortalToSubmarine::Interact_Implementation(AActor* InstigatorActor)
{
	if (IsConditionMet() == false)
	{
		return;
	}

	Super::Interact_Implementation(InstigatorActor);

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(InstigatorActor);
	if (PlayerCharacter == nullptr)
	{
		LOGV(Warning, TEXT("PlayerCharacter == nullptr"));
		return;
	}

	TArray<AUnderwaterCharacter*> BoundCharacters = PlayerCharacter->GetBoundCharacters();
	for (AUnderwaterCharacter* BoundCharacter : BoundCharacters)
	{
		BoundCharacter->SetActorLocation(TaregetDestination->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
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
		APawn* Player = ADPlayerState->GetPawn();
		if (Player == nullptr || IsValid(Player) == false || Player->IsValidLowLevel() == false || Player->IsPendingKillPending())
		{
			LOGV(Error, TEXT("Not Valid Player, PlayeStateName : %s"), *ADPlayerState->GetName());
			continue;
		}

		ADPlayerState->GetPawn()->bAlwaysRelevant = true;
	}

	ForceNetUpdate();
	LOGV(Log, TEXT("Releveant On"));
}

bool APortalToSubmarine::IsConditionMet()
{
	bool bIsConditionMet = Super::IsConditionMet();
	if (bIsConditionMet == false)
	{
		return false;
	}

	AADInGameMode* GM = Cast<AADInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM == nullptr)
	{
		LOGVN(Warning, TEXT("GM == nullptr"));
		return false;
	}

	return GM->IsAllPhaseCleared();
}
