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
#include "Projectile/GenericPool.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(ProjectileLog);

// Sets default values
AADProjectileBase::AADProjectileBase() : 
    TrailEffect(nullptr),
    Damage(100.0f), 
    bWasHit(false)
{
	PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    CollisionComponent->InitSphereRadius(5.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;
   
    TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
    TrailEffect->SetupAttachment(RootComponent);
    TrailEffect->SetAutoActivate(false);

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovementComp->bAutoActivate = false;

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AADProjectileBase::OnOverlapBegin);

    bReplicates = true;
    SetReplicateMovement(true);
}

void AADProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AADProjectileBase::Activate()
{
	Super::Activate();
    LOGP(Warning, TEXT("Activate"));
	GetWorld()->GetTimerManager().SetTimer(LifeTimerHandle, this, &AADProjectileBase::Deactivate, 10.0f, false);
    ProjectileMovementComp->SetActive(true);
    TrailEffect->Activate();
}

void AADProjectileBase::Deactivate()
{
	Super::Deactivate();
    LOGP(Warning, TEXT("Deactivate"));
    if(ObjectPool)
        ObjectPool->ReturnObject();
	GetWorld()->GetTimerManager().ClearTimer(LifeTimerHandle);
    SetOwner(nullptr);
    ProjectileMovementComp->SetActive(false);
    TrailEffect->Deactivate();
}

void AADProjectileBase::InitializeTransform(const FVector& Location, const FRotator& Rotation)
{
	SetActorLocation(Location);
	SetActorRotation(Rotation);
}

void AADProjectileBase::InitializeSpeed(const FVector& ShootDirection, const uint32 Speed)
{
    ProjectileMovementComp->InitialSpeed = Speed;
    ProjectileMovementComp->MaxSpeed = Speed;
    ProjectileMovementComp->Velocity = ShootDirection * Speed;
    ProjectileMovementComp->Activate(true);
}



