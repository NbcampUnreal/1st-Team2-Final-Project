#include "Boss/Serpmare/Serpmare.h"

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
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
}
