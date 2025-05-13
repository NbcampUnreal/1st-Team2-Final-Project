// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ADProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbyssDiverUnderWorld.h"

// Sets default values
AADProjectileBase::AADProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    CollisionComponent->InitSphereRadius(5.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;
   

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovementComp->InitialSpeed = 3000.0f;
    ProjectileMovementComp->MaxSpeed = 1000.0f;
    ProjectileMovementComp->bRotationFollowsVelocity = true;

    CollisionComponent->OnComponentHit.AddDynamic(this, &AADProjectileBase::OnHit);

}

void AADProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AADProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    LOG(TEXT("OnHitStart"));
    if (OtherActor && OtherActor != this && OtherComp)
    {
        UGameplayStatics::ApplyPointDamage(
            OtherActor,
            Damage,
            GetActorForwardVector(),
            Hit,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );
        LOG(TEXT("Hit"));

        Destroy();
    }
}


