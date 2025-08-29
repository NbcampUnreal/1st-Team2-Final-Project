#include "Monster/Boss/Serpmare/Serpmare.h"

ASerpmare::ASerpmare()
{
	LowerBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerBodyMesh"));
	LowerBodyMesh->SetupAttachment(GetMesh());

	WeakPointMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeakPointMesh"));
	WeakPointMesh->SetupAttachment(GetMesh());
}

void ASerpmare::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ASerpmare::OnMeshOverlapBegin);
}

void ASerpmare::Attack()
{
	Super::Attack();

	GetWorldTimerManager().SetTimer(AttackIntervalTimer, this, &ASerpmare::InitAttackInterval, AttackInterval, false);
	bCanAttack = false;
}

void ASerpmare::InitAttackInterval()
{
	bCanAttack = true;
}
