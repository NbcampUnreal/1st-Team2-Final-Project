// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/GobleFish/GFProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraCommon.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


AGFProjectile::AGFProjectile() : Damage(50.0f), LifeSpan(5.0f)
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneComponent);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(SceneComponent);
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetNotifyRigidBodyCollision(true); // Must be present for OnComponentHit to occur
	CollisionComponent->OnComponentHit.AddDynamic(this, &AGFProjectile::OnProjectileHit);


	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 1000.0f;
	ProjectileMovementComp->MaxSpeed = 1000.0f;
	ProjectileMovementComp->ProjectileGravityScale = 0.0f;
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->bAutoActivate = true;

	InitialLifeSpan = LifeSpan;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AGFProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Play TrailEffect on all clients, Because BeginPlay is called on both server + client.
	if (TrailEffect)
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailEffect,
			SceneComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true // Auto Destroy
		);
	}
}


void AGFProjectile::FireDirection(const FVector& ShootDirection)
{
	ProjectileMovementComp->Velocity = ShootDirection * ProjectileMovementComp->InitialSpeed;
}

void AGFProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
	{
		UGameplayStatics::ApplyDamage(
			OtherActor, // DamagedActor : AActor*
			Damage, // BaseDamage : float
			GetInstigatorController(), // Event Instigator Controller : AController*
			this, // Damage Causer : AActor*
			nullptr // Damage Type Class : TSubclassof<UDamageType>
			);
	}

	Destroy();
}



