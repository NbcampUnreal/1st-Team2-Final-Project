#include "Boss/Kraken/Kraken.h"
#include "AbyssDiverUnderWorld.h"
#include "Components/CapsuleComponent.h"

AKraken::AKraken()
{
	BiteAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Attack Collision"));
	BiteAttackCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteAttackCollision->SetCapsuleHalfHeight(80.0f);
	BiteAttackCollision->SetCapsuleRadius(80.0f);
	BiteAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteAttackCollision->ComponentTags.Add(TEXT("Bite Attack Collision"));

	TakeDownAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TakeDown Attack Collision"));
	TakeDownAttackCollision->SetupAttachment(GetMesh(), TEXT("TakeDownSocket"));
	TakeDownAttackCollision->SetCapsuleHalfHeight(80.0f);
	TakeDownAttackCollision->SetCapsuleRadius(80.0f);
	TakeDownAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TakeDownAttackCollision->ComponentTags.Add(TEXT("TakeDown Attack Collision"));

	WieldAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Wield Attack Collision"));
	WieldAttackCollision->SetupAttachment(GetMesh(), TEXT("WieldSocket"));
	WieldAttackCollision->SetCapsuleHalfHeight(80.0f);
	WieldAttackCollision->SetCapsuleRadius(80.0f);
	WieldAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WieldAttackCollision->ComponentTags.Add(TEXT("Wield Attack Collision"));
}

void AKraken::BeginPlay()
{
	Super::BeginPlay();
	
	BiteAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	TakeDownAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	WieldAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	
}

void AKraken::OnDeath()
{
	Super::OnDeath();

	SetEmissiveTransition();
}

void AKraken::Move()
{
	Super::Move();
	
	if (CurrentPatrolPointIndex >= PatrolPoints.Num())
	{
		CurrentPatrolPointIndex = 0;
	}
	
	const FVector TargetLocation = PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
	FlyingMoveToTarget(TargetLocation);

	++CurrentPatrolPointIndex;
}

void AKraken::MoveStop()
{
	Super::MoveStop();
	FlyingMoveToTargetStop();
}

void AKraken::MoveToTarget()
{
	Super::MoveToTarget();
	if (!IsValid(TargetPlayer)) return;
	FlyingMoveToTarget(TargetPlayer->GetActorLocation());
}

void AKraken::MoveToLastDetectedLocation()
{
	Super::MoveToLastDetectedLocation();
	FlyingMoveToTarget(LastDetectedLocation);
}
