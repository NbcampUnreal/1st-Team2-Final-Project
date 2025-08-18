// Fill out your copyright notice in the Description page of Project Settings.


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
	if (!ProjectileClass) return;

	if (!AIController || !BlackboardComponent) return;
	AActor* LockOnActor = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetActor"));
	if (!LockOnActor) return;

	FVector FireLocation = GetMesh()->GetSocketLocation("ProjectileSocket");
	FVector TargetActorLocation = LockOnActor->GetActorLocation();
	FVector ProjectileDirection = (TargetActorLocation - FireLocation).GetSafeNormal();
	FRotator FireRotation = ProjectileDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; // return AActor*
	SpawnParams.Instigator = GetInstigator(); // return APawn*

	AGFProjectile* Projectile = GetWorld()->SpawnActor<AGFProjectile>(ProjectileClass, FireLocation, FireRotation, SpawnParams);

	if (Projectile)
	{
		LOG(TEXT("Projectile Spawn Success!!!"))
		Projectile->FireDirection(ProjectileDirection);
	}
}

void AGobleFish::PlayAttackMontage()
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
