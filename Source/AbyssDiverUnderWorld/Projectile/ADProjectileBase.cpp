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
	TrailEffect->SetIsReplicated(true);

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovementComp->bAutoActivate = false;
	ProjectileMovementComp->SetIsReplicated(true);

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AADProjectileBase::OnOverlapBegin);

    bReplicates = true;
    SetReplicateMovement(true);
    SetNetDormancy(DORM_Awake);
}


void AADProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        if (IsValid(TrailEffect) &&
            TrailEffect->GetAsset() &&
            TrailEffect->IsRegistered())
        {
            TrailEffect->SetForceSolo(true);
            TrailEffect->Activate();
        }
    }
}

void AADProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AADProjectileBase::M_EffectActivate_Implementation(bool bActivate)
{
    LOGP(Warning, TEXT("bReplicates : %d"), GetIsReplicated());
    if (!TrailEffect)
    {
        LOGP(Warning, TEXT("TrailEffect is Null"));
        return;
    }
    if (bActivate)
    {
        TrailEffect->Activate();
        LOGP(Warning, TEXT("TrailEffectActivate"));
    }
    else
    {
        TrailEffect->Deactivate();
        LOGP(Warning, TEXT("TrailEffectDeactivate"));
    }
}

void AADProjectileBase::Activate()
{
	Super::Activate();
    LOGP(Warning, TEXT("Activate"));
    if (HasAuthority())
    {
        ProjectileMovementComp->SetActive(true);

        float DeactivateDelay = 10.0f;
	    GetWorld()->GetTimerManager().SetTimer(LifeTimerHandle, this, &AADProjectileBase::Deactivate, DeactivateDelay, false);
        TrailEffect->SetForceSolo(true);
        TrailEffect->Activate();
    }
}

void AADProjectileBase::Deactivate()
{
	Super::Deactivate();
    LOGP(Warning, TEXT("Deactivate"));

    if (!TrailEffect->IsActive())
    {
        M_EffectActivate(false);
        TrailEffect->Deactivate();
    }

    if(ObjectPool)
        ObjectPool->ReturnObject();
	GetWorld()->GetTimerManager().ClearTimer(LifeTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TrailDeactivateTimerHandle);
    SetOwner(nullptr);
    ProjectileMovementComp->SetActive(false);
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



