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
    TrailEffect(nullptr), 
    Damage(100.0f), 
    bWasHit(false)
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

    TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
    TrailEffect->SetupAttachment(CollisionComponent);
    TrailEffect->SetAutoActivate(false);

    bReplicates = true;
    SetReplicateMovement(true);
}

void AADProjectileBase::BeginPlay()
{
	Super::BeginPlay();


}

void AADProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bWasHit)
    {
        if (OtherActor && OtherActor != this && OtherComp && OtherComp->GetOwner() != this)
        {
            UGameplayStatics::ApplyPointDamage(
                OtherActor,
                Damage,
                GetActorForwardVector(),
                SweepResult,
                GetInstigatorController(),
                GetOwner(),
                UDamageType::StaticClass()
            );
            LOGP(Warning, TEXT("Hit %s"), *OtherActor->GetName());
        }
		TrailEffect->Deactivate();
        ProjectileMovementComp->StopMovementImmediately();
        ProjectileMovementComp->Deactivate();
        bWasHit = true;
    }

    FTimerHandle DestroyTimerHandle;
    float DestroyDelay = 10.0f;

    FTimerDelegate TimerDel;
    TimerDel.BindLambda([this]() {
        Destroy();
        });

    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, TimerDel, DestroyDelay, false);
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


