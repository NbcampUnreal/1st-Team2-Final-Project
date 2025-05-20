// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ADProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Monster/Monster.h"

// Sets default values
AADProjectileBase::AADProjectileBase() : Damage(100.0f)
{
	PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    CollisionComponent->InitSphereRadius(5.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;
   

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovementComp->InitialSpeed = 2000.0f;
    ProjectileMovementComp->MaxSpeed = 2000.0f;
    ProjectileMovementComp->bRotationFollowsVelocity = true;
    ProjectileMovementComp->bAutoActivate = true;

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AADProjectileBase::OnOverlapBegin);

    bReplicates = true;
    SetReplicateMovement(true);
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
        ABoss* Boss = Cast<ABoss>(OtherActor);
        AMonster* Monster = Cast<AMonster>(OtherActor);
        if (Boss || Monster)
        {
            UGameplayStatics::ApplyPointDamage(
                OtherActor,
                Damage,
                GetActorForwardVector(),
                SweepResult,
                GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );
            LOG(TEXT("Hit"));
            LOG(TEXT("%s"), *OtherActor->GetName());

            Destroy();
        }
        else
        {
            LOG(TEXT("Is not Monster Actor"));
        }
    }
}

void AADProjectileBase::SetProjectileSpeed(float Speed)
{
    ProjectileMovementComp->InitialSpeed = Speed;
    ProjectileMovementComp->MaxSpeed = Speed;
}


