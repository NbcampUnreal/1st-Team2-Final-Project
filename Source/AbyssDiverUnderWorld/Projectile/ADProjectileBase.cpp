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

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AADProjectileBase::OnOverlapBegin);

}

void AADProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AADProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    LOG(TEXT("OnHitStart"));
    if (OtherActor && OtherActor != this && OtherComp)
    {
        UGameplayStatics::ApplyPointDamage(
            OtherActor,
            30.0f,
            GetActorForwardVector(),
            SweepResult,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );
        LOG(TEXT("Hit"));

        Destroy();
    }
}


