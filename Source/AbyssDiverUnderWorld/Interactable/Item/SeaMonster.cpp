#include "Interactable/Item/SeaMonster.h"
#include "AbyssDiverUnderWorld.h"

ASeaMonster::ASeaMonster()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComponent->SetupAttachment(SceneComponent);

	StaticMeshComponent->SetCollisionProfileName("BlockAllDynamic");
	StaticMeshComponent->SetGenerateOverlapEvents(true);
	StaticMeshComponent->SetSimulatePhysics(true);
}

void ASeaMonster::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		MorphInto();
	}
}

void ASeaMonster::MorphInto()
{
	LOG(TEXT("Interact!!"));
}
