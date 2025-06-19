#include "CurrentZone.h"
#include "Character/UnderwaterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ACurrentZone::ACurrentZone()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
    RootComponent = TriggerZone;

    TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerZone->SetCollisionObjectType(ECC_WorldStatic);
    TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerZone->SetGenerateOverlapEvents(true);

    DeepTriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DeepTriggerZone"));
    DeepTriggerZone->SetupAttachment(RootComponent);

    DeepTriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DeepTriggerZone->SetCollisionObjectType(ECC_WorldStatic);
    DeepTriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    DeepTriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DeepTriggerZone->SetGenerateOverlapEvents(true);

    if (TriggerZone->OnComponentBeginOverlap.IsBound())
    {
        return;
    }

    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACurrentZone::OnOverlapBegin);
    TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ACurrentZone::OnOverlapEnd);

    DeepTriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACurrentZone::OnDeepTriggerOverlapBegin);
    DeepTriggerZone->OnComponentEndOverlap.AddDynamic(this, &ACurrentZone::OnDeepTriggerOverlapEnd);
}

void ACurrentZone::BeginPlay()
{
    Super::BeginPlay();

    // PushDirection이 0일 경우 대비
    if (PushDirection.IsNearlyZero())
    {
        PushDirection = FVector(1.f, 0.f, 0.f);
    }
}

void ACurrentZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Add(Character, false);
        
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (!OriginalSpeeds.Contains(Character))
            {
                OriginalSpeeds.Add(Character, Movement->MaxWalkSpeed);
                OriginalAccelerations.Add(Character, Movement->MaxAcceleration);
            }

            // 무조건 타이머 재시작 (중복 등록 방지용 IsTimerActive 제거)
            GetWorldTimerManager().SetTimer(CurrentForceTimer, this, &ACurrentZone::ApplyCurrentForce, 0.05f, true);
        }
    }
}

void ACurrentZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Remove(Character);

        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (OriginalSpeeds.Contains(Character))
            {
                Movement->MaxWalkSpeed = OriginalSpeeds[Character];
                OriginalSpeeds.Remove(Character);
            }
            if (OriginalAccelerations.Contains(Character))
            {
                Movement->MaxAcceleration = OriginalAccelerations[Character];
                OriginalAccelerations.Remove(Character);
            }
        }

        if (AffectedCharacters.Num() == 0)
        {
            GetWorldTimerManager().ClearTimer(CurrentForceTimer);
        }

        FTimerHandle RecoverMovementTimer;
        GetWorld()->GetTimerManager().SetTimer(RecoverMovementTimer, [Character]()
            {
                if (IsValid(Character) && Character->IsPendingKillPending() == false && Character->IsValidLowLevel() && Character->GetCharacterMovement())
                {
                    UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
                    if (IsValid(CharacterMovement) == false || CharacterMovement->IsValidLowLevel() == false)
                    {
                        return;
                    }

                    CharacterMovement->SetMovementMode(MOVE_Swimming);
                }
            }, 2.0f, false);
    }
}

void ACurrentZone::OnDeepTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        if (AffectedCharacters.Contains(Character) == false)
        {
            return;
        }

        AffectedCharacters[Character] = true;
        LOGV(Log, TEXT(" %s is In DeepCurrent"), *Character->GetName());
    }
}

void ACurrentZone::OnDeepTriggerOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        if (AffectedCharacters.Contains(Character) == false)
        {
            return;
        }

        AffectedCharacters[Character] = false;
        LOGV(Log, TEXT(" %s is Out of DeepCurrent"), *Character->GetName());
    }
}

void ACurrentZone::ApplyCurrentForce()
{
    for (TPair<TObjectPtr<AUnderwaterCharacter>, bool>& CharacterPair : AffectedCharacters)
    {
        AUnderwaterCharacter* Character = CharacterPair.Key;

        if (!Character || !Character->GetCharacterMovement())
            continue;

        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

        if (Movement->IsFalling() || Movement->MovementMode == MOVE_None)
        {
            Movement->SetMovementMode(MOVE_Swimming);
        }

        // 캐릭터 속도 0.1배 만들기 필요
        bool bIsInDeepCurrentZone = CharacterPair.Value;
        if (bIsInDeepCurrentZone)
        {
            Character->SetZoneSpeedMultiplier(0.1f);
            LOGV(Log, TEXT(" %s : 0.1"), *Character->GetName());
        }
        else
        {
            Character->SetZoneSpeedMultiplier(1.0f);
            LOGV(Log, TEXT(" %s : 1.0"), *Character->GetName());
        }

        FVector PushDir = PushDirection.GetSafeNormal();
        float FinalFlowStrength = FlowStrength;

        const FVector FlowForce = PushDir * FinalFlowStrength * Movement->GetMaxAcceleration() * GetWorld()->DeltaTimeSeconds;

        Movement->Velocity += FlowForce;
    }
}
