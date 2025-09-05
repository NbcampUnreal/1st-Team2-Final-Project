#include "Monster/GobleFish/GobleFish.h"
#include "Monster/GobleFish/GFProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbyssDiverUnderWorld.h"

AGobleFish::AGobleFish()
{
	PrimaryActorTick.bCanEverTick = true;

	GobleFishHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GobleFishHitSphere"));
	GobleFishHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	GobleFishHitSphere->InitSphereRadius(20.0f);
	GobleFishHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GobleFishHitSphere->SetHiddenInGame(true);
}

void AGobleFish::FireProjectile()
{
	if (!ProjectileClass)
	{
		LOGV(Error, TEXT("ProjectileClass is not set in GobleFish"));
		return;
	}

	if (!AIController || !BlackboardComponent)
	{
		LOGV(Error, TEXT("AIController or BlackboardComponent is nullptr in GobleFish"));
		return;
	}

	AActor* LockOnActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(/*"TargetActor"*/"TargetPlayer"));
	if (!LockOnActor)
	{
		LOGV(Error, TEXT("LockOnActor is nullptr in GobleFish"));
		return;
	}

	FVector FireLocation = GetMesh()->GetSocketLocation("ProjectileSocket");
	FVector TargetActorLocation = LockOnActor->GetActorLocation();
	FVector ProjectileDirection = (TargetActorLocation - FireLocation).GetSafeNormal();
	FRotator FireRotation = ProjectileDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; // return AActor*
	SpawnParams.Instigator = GetInstigator(); // return APawn*

	AGFProjectile* Projectile = GetWorld()->SpawnActor<AGFProjectile>(ProjectileClass, FireLocation, FireRotation, SpawnParams);
	if (Projectile == nullptr)
	{
		LOG(TEXT("Projectile Spawn Failed!!!"));
		return;
	}

	Projectile->FireDirection(ProjectileDirection);
}

void AGobleFish::Attack()
{
	// Initialize at Monster class BeginPlay
	if (!AIController) return;
	if (!BlackboardComponent) return;
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	// If any montage is playing, prevent duplicate playback
	if (AnimInst->IsAnyMontagePlaying()) return;


	if (BlackboardComponent->GetValueAsBool("bInMeleeRange"))
	{
		if (AttackAnimations.Num() > 0)
		{
			const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);

			if (IsValid(AttackAnimations[AttackType]))
			{
				M_PlayMontage(AttackAnimations[AttackType]);
			}
		}
	}
	else if (BlackboardComponent->GetValueAsBool("bInRangedRange"))
	{
		if (RangedAttackAnimations.Num() > 0)
		{
			const uint8 AttackType = FMath::RandRange(0, RangedAttackAnimations.Num() - 1);

			if (IsValid(RangedAttackAnimations[AttackType]))
			{
				M_PlayMontage(RangedAttackAnimations[AttackType]);
			}
		}
	}
	else return;
}

