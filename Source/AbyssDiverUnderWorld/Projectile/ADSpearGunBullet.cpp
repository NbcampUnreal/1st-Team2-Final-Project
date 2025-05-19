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

AADSpearGunBullet::AADSpearGunBullet() : BulletType(ESpearGunType::Basic), AdditionalDamage(0), PoisonDuration(0), bWasHit(false)
{
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkeletalMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

    TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
    TrailEffect->SetupAttachment(RootComponent);

    TrailEffect->bAutoActivate = false;

    Damage = 450.0f;
}

void AADSpearGunBullet::OnRep_BulletType()
{

}

void AADSpearGunBullet::BeginPlay()
{
    Super::BeginPlay();
    //3가지 타입의 Bullet BP를 만들거면 여기 하나의 BP만 만들고 속성을 바꿀 거면 OnRep
    FString EnumName = StaticEnum<ESpearGunType>()->GetNameStringByValue((int64)ESpearGunType::Basic);
    FString RowNameString = EnumName + "SpearGunBullet";
    FName ProjectileName(*RowNameString);

    if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
    {
        DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
    }
    if (DataTableSubsystem)
    {
        FFADProjectileDataRow* ProjectileInfo = DataTableSubsystem->GetProjectileDataArrayByName(ProjectileName);
        if (ProjectileInfo)
        {
            AdditionalDamage = ProjectileInfo->AdditionalDamage;
            // 로드되어 있으면 가져오고 안 되어 있으면 동기 로드
            UNiagaraSystem* Effect = ProjectileInfo->TrailEffect.IsValid() ? ProjectileInfo->TrailEffect.Get() : ProjectileInfo->TrailEffect.LoadSynchronous();
            TrailEffect->SetAsset(Effect);
            TrailEffect->Activate(true);
        }
    }
    //TODO : Shoot Sound
    //TODO : Shoot Effect

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
    //TODO : Hit Sound
    switch (BulletType)
    {
    case ESpearGunType::Basic:
        LOG(TEXT("BasicBullet"));
        break;
    case ESpearGunType::Bomb:
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
    //TODO : Hit Effect
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
    //TODO : Hit Effect
    //TODO : 디버프 컴포넌트 함수 호출
    LOG(TEXT("PoisonBullet"));
}

