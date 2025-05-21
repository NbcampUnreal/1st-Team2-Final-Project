#include "Boss/SeaObserver/SeaObserver.h"
#include "Components/CapsuleComponent.h"

ASeaObserver::ASeaObserver()
{
	BiteCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BiteCollision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteCollision->SetCapsuleHalfHeight(50.0f);
	BiteCollision->SetCapsuleRadius(50.0f);
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->ComponentTags.Add(TEXT("BiteCollision"));
}

void ASeaObserver::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);

	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
}

void ASeaObserver::OnDeath()
{
	Super::OnDeath();

	GetMesh()->OnComponentBeginOverlap.RemoveAll(this);
	BiteCollision->OnComponentBeginOverlap.RemoveAll(this);
}
