#include "SpikeHazard.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"                  
#include "Character/UnderwaterCharacter.h" 
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"

ASpikeHazard::ASpikeHazard()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
    CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionBox->SetGenerateOverlapEvents(true);
    CollisionBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));

    SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
    SpikeMesh->SetupAttachment(RootComponent);
    SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASpikeHazard::OnOverlapBegin);
}

void ASpikeHazard::BeginPlay()
{
    Super::BeginPlay();
}

void ASpikeHazard::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor);
    if (Character != nullptr)
    {
        // 1. 데미지 적용
        UGameplayStatics::ApplyDamage(Character, DamageAmount, nullptr, this, nullptr);

        // 2. 넉백 적용
        FVector KnockbackDir = Character->GetActorLocation() - GetActorLocation();
        KnockbackDir.Z = 0.f;
        KnockbackDir.Normalize();

        FVector LaunchVelocity = KnockbackDir * KnockbackStrength;
        LaunchVelocity *= 0.5f;        
        LaunchVelocity.Z = 120.f;     

        Character->LaunchCharacter(LaunchVelocity, true, true);

        // 3. 일정 시간 뒤 수영 모드로 복구
        FTimerHandle RecoverTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(RecoverTimerHandle, [Character]()
            {
                if (Character && Character->GetCharacterMovement())
                {
                    Character->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
                }
            }, 0.5f, false);
    }
}
