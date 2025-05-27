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
#include "DataRow/FADProjectileDataRow.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AADSpearGunBullet::AADSpearGunBullet() : 
	StaticMesh(nullptr),
	DataTableSubsystem(nullptr),
    BurstEffect(nullptr),
    BulletType(ESpearGunType::MAX), 
    AdditionalDamage(0), 
    PoisonDuration(0),
    bWasAdditionalDamage(false)
{
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkeletalMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

    Damage = 450.0f;

    ConstructorHelpers::FObjectFinder<UNiagaraSystem> BurstEffectFinder(TEXT("/Game/SurvivalFX/Particles/Explosion/PS_Explosion_Smokey_Small"));

    if (BurstEffectFinder.Succeeded())
    {
        BurstEffect = BurstEffectFinder.Object;
    }
}

void AADSpearGunBullet::OnRep_BulletType()
{
    switch (BulletType)
    {
    case ESpearGunType::Basic:
        ProjectileName = "BasicSpearGunBullet";
        break;
    case ESpearGunType::Bomb:
        ProjectileName = "BombSpearGunBullet";
        break;
    case ESpearGunType::Poison:
        ProjectileName = "PoisonSpearGunBullet";
        break;
    default:
        break;
    }
    LOGP(Warning, TEXT("%s"), *ProjectileName.ToString());
    if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
    {
        DTSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
    }
    if (DTSubsystem)
    {
        FFADProjectileDataRow* ProjectileInfo = DTSubsystem->GetProjectileDataArrayByName(ProjectileName);
        if (ProjectileInfo)
        {
            AdditionalDamage = ProjectileInfo->AdditionalDamage;
            // 로드되어 있으면 가져오고 안 되어 있으면 동기 로드
            UNiagaraSystem* Effect = ProjectileInfo->TrailEffect.IsValid() ? ProjectileInfo->TrailEffect.Get() : ProjectileInfo->TrailEffect.LoadSynchronous();
            TrailEffect->SetAsset(Effect);
            TrailEffect->Activate(true);
        }
    }
}

void AADSpearGunBullet::BeginPlay()
{
    Super::BeginPlay();
    //TODO : Shoot Sound
    //TODO : Shoot Effect
}

void AADSpearGunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (!bWasAdditionalDamage)
    {
        if (OtherActor && OtherActor != this && OtherComp && OtherComp->GetOwner() != this)
        {
            AttachToHitActor(OtherComp, SweepResult, true);
            ApplyAdditionalDamage();
            bWasAdditionalDamage = true;
        }
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

    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel3));

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

                    LOGP(Warning, TEXT("Unique hit: %s"), *HitActor->GetName());

                    // 여기에 데미지 주기나 처리 로직 작성
                    UGameplayStatics::ApplyPointDamage(
                        HitActor,
                        AdditionalDamage,
                        GetActorForwardVector(),
                        Hit,
                        GetInstigatorController(),
                        GetOwner(),
                        UDamageType::StaticClass()
                    );

                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		                GetWorld(),
                        BurstEffect,
                        Hit.Location,
		                FRotator::ZeroRotator,
		                FVector(1.0f),
		                true,  // bAutoDestroy
		                true,  // bAutoActivate
		                ENCPoolMethod::None,
		                true   // bPreCullCheck
	                );
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
    //TODO : Hit Effect
    //TODO : 디버프 컴포넌트 함수 호출
    LOGP(Warning, TEXT("Addict"));
}

