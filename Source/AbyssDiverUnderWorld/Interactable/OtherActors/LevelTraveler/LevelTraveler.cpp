#include "Interactable/OtherActors/LevelTraveler/LevelTraveler.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADInGameMode.h"

#include "Kismet/GameplayStatics.h"

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
	AADInGameMode* GM = Cast<AADInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM == nullptr)
	{
		LOGVN(Warning, TEXT("GM == nullptr"));
		return;
	}

	if (IsConditionMet() == false)
	{
		LOGVN(Warning, TEXT("Condition Is not Met"));
		return;
	}

	GM->ReadyForTravelToCamp();
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
