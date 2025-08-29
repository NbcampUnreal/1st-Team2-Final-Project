#include "Monster/Boss/AlienShark/AlienShark.h"
#include "Components/CapsuleComponent.h"



AAlienShark::AAlienShark()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BiteCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Collision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteCollision->SetCapsuleHalfHeight(80.0f);
	BiteCollision->SetCapsuleRadius(80.0f);
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->ComponentTags.Add(TEXT("Bite Collision"));
}

void AAlienShark::BeginPlay()
{
	Super::BeginPlay();

	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &AAlienShark::OnMeshOverlapBegin);
}
