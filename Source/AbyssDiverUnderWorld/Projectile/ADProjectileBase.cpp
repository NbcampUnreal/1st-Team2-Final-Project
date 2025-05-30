// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ADProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Monster/Monster.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"

DEFINE_LOG_CATEGORY(ProjectileLog);

// Sets default values
AADProjectileBase::AADProjectileBase() : 
    Damage(100.0f), 
    bWasHit(false)
{
	PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    CollisionComponent->InitSphereRadius(5.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;
   

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    //ProjectileMovementComp->bRotationFollowsVelocity = true;
    ProjectileMovementComp->bAutoActivate = true;

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AADProjectileBase::OnOverlapBegin);

    bReplicates = true;
    SetReplicateMovement(true);
}

void AADProjectileBase::BeginPlay()
{
	Super::BeginPlay();

    ProjectileMovementComp->InitialSpeed = 1000.0f;
    ProjectileMovementComp->MaxSpeed = 1500.0f;
}

void AADProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AADProjectileBase::SetProjectileSpeed(float Speed)
{
    ProjectileMovementComp->InitialSpeed = Speed;
    ProjectileMovementComp->MaxSpeed = Speed;
}

void AADProjectileBase::SetProjectileName(FName Name)
{
    ProjectileName = Name;
}


