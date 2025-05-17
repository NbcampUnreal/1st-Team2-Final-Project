// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ADSpearGunBullet.h"
#include "Kismet/GameplayStatics.h"
#include "AbyssDiverUnderWorld.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"  
#include "GameFramework/ProjectileMovementComponent.h"

AADSpearGunBullet::AADSpearGunBullet() : BulletType(ESpearGunType::Basic), AdditionalDamage(0), PoisonDuration(0), bWasHit(false)
{
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkeletalMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

    Damage = 10.0f; //추후 450.0f
}

void AADSpearGunBullet::OnRep_BulletType()
{
    switch (BulletType)
    {
    case ESpearGunType::Basic:
        break;
    case ESpearGunType::Bomb:
        AdditionalDamage = 20.0f;//200.0f;
        break;
    case ESpearGunType::Poison:
        AdditionalDamage = 30.0f;//750.0f;
        PoisonDuration = 4;
        break;
    default:
        break;
    }
}

void AADSpearGunBullet::BeginPlay()
{
    Super::BeginPlay();
    LOG(TEXT("UpdatedComponent is %s"),ProjectileMovementComp->UpdatedComponent ? *ProjectileMovementComp->UpdatedComponent->GetName() : TEXT("NULL"));

}

void AADSpearGunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
                this,
                UDamageType::StaticClass()
            );
            LOG(TEXT("Hit %s"), *OtherActor->GetName());

            ApplyAdditionalDamage();
        }
        ProjectileMovementComp->StopMovementImmediately();
        ProjectileMovementComp->Deactivate();
        bWasHit = true;
    }
}

void AADSpearGunBullet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AADSpearGunBullet, BulletType);
}

void AADSpearGunBullet::ApplyAdditionalDamage()
{
    FTimerHandle BurstTimerHandle;
    switch (BulletType)
    {
    case ESpearGunType::Basic:
        LOG(TEXT("BasicBullet"));
        break;
    case ESpearGunType::Bomb:
        //TODO : 삐삐삐 소리
        LOG(TEXT("BombTimer"));
        GetWorld()->GetTimerManager().SetTimer(BurstTimerHandle, this, &AADSpearGunBullet::Burst, 2.5f, false);
        
        break;
    case ESpearGunType::Poison:
        Addict();
        break;
    default:
        break;
    }
}

void AADSpearGunBullet::Burst()
{
    //TODO : 터지는 이펙트
    LOG(TEXT("BombBullet"));

    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    FVector Start = GetActorLocation();
    FVector End = Start;
    float Radius = 200.0f;

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        Start,
        End,
        Radius,
        ObjectTypes,
        false,
        IgnoreActors, // 무시할 액터
        EDrawDebugTrace::ForDuration,
        HitResults,
        true
    );
    TSet<AActor*> UniqueActors;
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (!UniqueActors.Contains(HitActor))
                {
                    UniqueActors.Add(HitActor);

                    LOGN(TEXT("Unique hit: %s"), *HitActor->GetName());

                    // 여기에 데미지 주기나 처리 로직 작성
                }
            }
        }
    }
    else 
    {
        LOG(TEXT("No Hit in Bomb Area"));
    }
}

void AADSpearGunBullet::Addict()
{
    //TODO : 디버프 컴포넌트 함수 호출
    LOG(TEXT("PoisonBullet"));
}

