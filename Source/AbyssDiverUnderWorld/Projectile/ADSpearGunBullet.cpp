// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/ADSpearGunBullet.h"
#include "Kismet/GameplayStatics.h"
#include "AbyssDiverUnderWorld.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"  
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "DataRow/FADProjectileDataRow.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "AbyssDiverUnderWorld.h"

AADSpearGunBullet::AADSpearGunBullet() : 
	StaticMesh(nullptr),
	DataTableSubsystem(nullptr),
    BulletType(ESpearGunType::MAX), 
    AdditionalDamage(0), 
    PoisonDuration(0),
    bWasAdditionalDamage(false)
{
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkeletalMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

    Damage = 450.0f;

}

void AADSpearGunBullet::M_SpawnFX_Implementation(UNiagaraSystem* Effect, ESFX SFXType, const FVector& SpawnLocation)
{
    if(SFXType != ESFX::Max)
        SoundSubsystem->PlayAt(SFXType, SpawnLocation);
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        Effect,
        SpawnLocation,
        FRotator::ZeroRotator,
        FVector(1.0f),
        true,  // bAutoDestroy
        true,  // bAutoActivate
        ENCPoolMethod::None,
        true   // bPreCullCheck
    );
}

void AADSpearGunBullet::M_AdjustTransform_Implementation(FTransform WorldTransform)
{
    SetActorTransform(WorldTransform);
}

void AADSpearGunBullet::OnRep_BulletType()
{
}

void AADSpearGunBullet::BeginPlay()
{
    Super::BeginPlay();
    if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
    {
        DTSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
        SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
    }

    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Bullet spawned. Bullet Id : %d"), GetProjectileId());
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Bullet replicated. Bullet Id : %d"), GetProjectileId());
}

void AADSpearGunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bWasHit)
    {
        bWasHit = true;
        TrailEffect->Deactivate();
        if (OtherActor && OtherActor != this && OtherComp && OtherComp->GetOwner() != this)
        {
            SoundSubsystem->PlayAt(ESFX::Hit, SweepResult.Location);
            if (HasAuthority())
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
                AttachToHitActor(OtherComp, SweepResult, true);
                M_AdjustTransform(GetActorTransform()); //위치 보정
            }
            LOGP(Warning, TEXT("Hit Basic Damage To %s"), *OtherActor->GetName());
            ApplyAdditionalDamage();
        }
        LOGP(Warning, TEXT("SetProjectileMovementAttribute"));
        ProjectileMovementComp->StopMovementImmediately();
        ProjectileMovementComp->Deactivate();
    }

}

void AADSpearGunBullet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AADSpearGunBullet, BulletType);
}

void AADSpearGunBullet::AttachToHitActor(USceneComponent* HitComp, const FHitResult& Hit, bool bAttachOnHit)
{
    if (bAttachOnHit)
    {
        if (HitComp)
        {
            FName HitBone = Hit.BoneName;
            USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(HitComp);
            if (HitBone != NAME_None && SkeletalMesh)
            {

                FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);  // 상대 위치로 유지
                AttachToComponent(SkeletalMesh, AttachRules, HitBone);

                LOGP(Warning, TEXT("Attached to SkeletalMeshComponent at bone: %s HitComp : %s"), *HitBone.ToString(), *HitComp->GetName());
            }
            else
            {
                // 일반 메시나 컴포넌트에 부착
                StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                StaticMesh->SetCollisionProfileName("BlockAllDynamic");
                StaticMesh->SetSimulatePhysics(true);     // 중력 낙하
                StaticMesh->WakeAllRigidBodies();
            }
        }
        else
        {
            LOGP(Warning, TEXT("Hit Component is NULL"));
        }
    }
}

void AADSpearGunBullet::ApplyAdditionalDamage()
{
    FTimerHandle BurstTimerHandle;
    float TimerDuration = 2.5f; // 폭탄이 터지는 시간
    //TODO : Hit Sound
    switch (BulletType)
    {
    case ESpearGunType::Basic:
        LOGP(Warning, TEXT("BasicBullet"));
        break;
    case ESpearGunType::Bomb:
        LOGP(Warning, TEXT("BombBullet"));
        GetWorld()->GetTimerManager().SetTimer(BurstTimerHandle, this, &AADSpearGunBullet::Burst, TimerDuration, false);
        break;
    case ESpearGunType::Poison:
        LOGP(Warning, TEXT("PoisonBullet"));
        Addict();
        break;
    default:
        break;
    }
}

void AADSpearGunBullet::Burst()
{
    LOGP(Warning, TEXT("Burst"));
	UNiagaraSystem* BurstEffect = nullptr;
    if (DTSubsystem)
    {
        FFADProjectileDataRow* BombProjectileInfo = DTSubsystem->GetProjectileData(static_cast<uint8>(BulletType));
        if (BombProjectileInfo)
        {
            AdditionalDamage = BombProjectileInfo->AdditionalDamage;
            // 로드되어 있으면 가져오고 안 되어 있으면 동기 로드
            BurstEffect = BombProjectileInfo->HitEffect.IsValid() ? BombProjectileInfo->HitEffect.Get() : BombProjectileInfo->HitEffect.LoadSynchronous();
        }
    }

    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel3));

    FVector Start = GetActorLocation();
    FVector End = Start;
    float Radius = 200.0f;

    LOGP(Warning, TEXT("Start : %s, End : %s"), *Start.ToString(), *End.ToString());

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
        EDrawDebugTrace::None,
        HitResults,
        true
    );
    TSet<AActor*> UniqueActors;

    if (bHit && BurstEffect)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (!UniqueActors.Contains(HitActor))
                {
                    UniqueActors.Add(HitActor);

                    LOGP(Warning, TEXT("Unique hit: %s"), *HitActor->GetName());
                    // 여기에 데미지 주기나 처리 로직 작성
                    if (HasAuthority())
                    {
                        UGameplayStatics::ApplyPointDamage(
                            HitActor,
                            AdditionalDamage,
                            GetActorForwardVector(),
                            Hit,
                            GetInstigatorController(),
                            GetOwner(),
                            UDamageType::StaticClass()
                        );
                        LOGP(Warning, TEXT("AdditionalDamage By Bomb : %d"), AdditionalDamage);
                            M_SpawnFX(BurstEffect, ESFX::Explosion, Hit.Location);
                    }
                }
            }
        }
    }
    else 
    {
        LOGP(Warning, TEXT("No Hit in Bomb Area"));
    }
}

void AADSpearGunBullet::Addict()
{
    //TODO : 디버프 컴포넌트 함수 호출
    if (HasAuthority())
    {
	    UNiagaraSystem* PoisonEffect = nullptr;
        if (DTSubsystem)
        {
            FFADProjectileDataRow* PoisonProjectileInfo = DTSubsystem->GetProjectileData(static_cast<uint8>(BulletType));
            if (PoisonProjectileInfo)
            {
                AdditionalDamage = PoisonProjectileInfo->AdditionalDamage;
                // 로드되어 있으면 가져오고 안 되어 있으면 동기 로드
                PoisonEffect = PoisonProjectileInfo->HitEffect.IsValid() ? PoisonProjectileInfo->HitEffect.Get() : PoisonProjectileInfo->HitEffect.LoadSynchronous();
            }
        }
        if (PoisonEffect)
        {
        
                M_SpawnFX(PoisonEffect, ESFX::Max, GetActorLocation());
        }
    }


    LOGP(Warning, TEXT("Addict"));
}

