#include "Interactable/OtherActors/LevelTraveler/LevelTraveler.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADInGameMode.h"
#include "Framework/ADCampGameMode.h"
#include "Character/UnderwaterCharacter.h"

ALevelTraveler::ALevelTraveler()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultComp = CreateDefaultSubobject<USceneComponent>(TEXT("Default Comp"));
	SetRootComponent(DefaultComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	MeshComp->SetupAttachment(RootComponent);

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("Interactable Comp"));

	bIsHoldMode = false;
}

void ALevelTraveler::BeginPlay()
{
	Super::BeginPlay();
}

void ALevelTraveler::Interact_Implementation(AActor* InstigatorActor)
{
	if (IsConditionMet() == false)
	{
		LOGVN(Warning, TEXT("Condition Is not Met"));
		return;
	}

	if (AADInGameMode* InGameMode = Cast<AADInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		InGameMode->ReadyForTravelToCamp();
	}
	else if (AADCampGameMode* CampGameMode = Cast<AADCampGameMode>(GetWorld()->GetAuthGameMode()))
	{
		AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorActor);
		if (UnderwaterCharacter == nullptr)
		{
			LOGV(Error, TEXT("UnderwaterCharacter == nullptr"));
			return;
		}

		if (UnderwaterCharacter->IsLocallyControlled() == false)
		{
			return;
		}

		CampGameMode->ServerTravelToInGameLevel();
	}
	else
	{
		LOGV(Error, TEXT("Failed to Travel Level"));
	}
}

bool ALevelTraveler::IsConditionMet()
{
	return true;
}

UADInteractableComponent* ALevelTraveler::GetInteractableComponent() const
{
	return InteractableComp;
}

bool ALevelTraveler::IsHoldMode() const
{
	return bIsHoldMode;
}

FString ALevelTraveler::GetInteractionDescription() const
{
	return TEXT("Go to Next Level!");
}
