#include "Projectile/ADFlareGunBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"

AADFlareGunBullet::AADFlareGunBullet()
{
	FlareMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlareMesh"));
	FlareMesh->SetupAttachment(RootComponent);
}

void AADFlareGunBullet::InitializeSpeed(const FVector& Dir, uint32 Speed)
{
	Super::InitializeSpeed(Dir, Speed);
}

void AADFlareGunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bWasHit) return;
	bWasHit = true;
	
	ProjectileMovementComp->StopMovementImmediately();
	AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform, SweepResult.BoneName);

	GetWorld()->GetTimerManager().SetTimer(HitDestroyHandle, this, &ThisClass::Deactivate, HitLifetime, false);
}
