#include "CurrentZone.h"

#include "Character/UnderwaterCharacter.h"

#include "Components/ArrowComponent.h"
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

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Current Direction Arrow"));
    ArrowComponent->SetupAttachment(RootComponent);

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
    if (PushDirection.IsNearlyZero() && bShouldUseArrowDirection == false)
    {
        PushDirection = FVector(1.f, 0.f, 0.f);
    }

    if (bShouldUseArrowDirection)
    {
        PushDirection = ArrowComponent->GetForwardVector();
    }
}

void ACurrentZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    LOGVN(Log, TEXT("Current OnOverlapBegin Begin"));
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Add(Character, false);
        LOGVN(Log, TEXT("AffectedCharacters Add(%s)"), *Character->GetName());
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (!OriginalSpeeds.Contains(Character))
            {
                OriginalSpeeds.Add(Character, Movement->MaxWalkSpeed);
                OriginalAccelerations.Add(Character, Movement->MaxAcceleration);
                LOGVN(Log, TEXT("Save Original Speed(%f) and Original Acceleration(%f) (%s)"), Movement->MaxWalkSpeed, Movement->MaxAcceleration, *Character->GetName());
            }

            // 무조건 타이머 재시작 (중복 등록 방지용 IsTimerActive 제거)
            GetWorldTimerManager().SetTimer(CurrentForceTimer, this, &ACurrentZone::ApplyCurrentForce, 0.05f, true);
        }
    }
}

void ACurrentZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    LOGVN(Log, TEXT("Current OverlapEnd Begin"));
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Remove(Character);
        LOGVN(Log, TEXT("AffectedCharacters Remove(%s)"), *Character->GetName());
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (OriginalSpeeds.Contains(Character))
            {
                Movement->MaxWalkSpeed = OriginalSpeeds[Character];
                OriginalSpeeds.Remove(Character);
                LOGVN(Log, TEXT("Restore to OriginalSpeeds : %f (%s)"), Movement->MaxWalkSpeed, *Character->GetName());
            }
            if (OriginalAccelerations.Contains(Character))
            {
                Movement->MaxAcceleration = OriginalAccelerations[Character];
                OriginalAccelerations.Remove(Character);
                LOGVN(Log, TEXT("Restore to OriginalAccelerations : %f (%s)"), Movement->MaxAcceleration, *Character->GetName());
            }
        }

        if (AffectedCharacters.Num() == 0)
        {
            GetWorldTimerManager().ClearTimer(CurrentForceTimer);
        }

        FTimerHandle RecoverMovementTimer;
        GetWorld()->GetTimerManager().SetTimer(RecoverMovementTimer, [Character, this]()
            {
                if (IsValid(Character) && Character->IsPendingKillPending() == false && Character->IsValidLowLevel() && Character->GetCharacterMovement())
                {
                    UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
                    if (IsValid(CharacterMovement) == false || CharacterMovement->IsValidLowLevel() == false)
                    {
                        return;
                    }
                    LOGVN(Log, TEXT("Set Movement mode to Swimming (%s)"), *Character->GetName());
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
    for (const TPair<TObjectPtr<AUnderwaterCharacter>, bool>& CharacterPair : AffectedCharacters)
    {
        AUnderwaterCharacter* Character = CharacterPair.Key;

        if (!Character || !Character->GetCharacterMovement())
            continue;

        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

        if (Movement->IsFalling() || Movement->MovementMode == MOVE_None)
        {
            Movement->SetMovementMode(MOVE_Swimming);
        }

        // // 캐릭터 속도 0.1배 만들기 필요
        // bool bIsInDeepCurrentZone = CharacterPair.Value;
        // if (bIsInDeepCurrentZone)
        // {
        //     Character->SetZoneSpeedMultiplier(0.1f);
        //     LOGV(Log, TEXT(" %s : 0.1"), *Character->GetName());
        // }
        // else
        // {
        //     Character->SetZoneSpeedMultiplier(1.0f);
        //     LOGV(Log, TEXT(" %s : 1.0"), *Character->GetName());
        // }

        FVector PushDir = PushDirection.GetSafeNormal();
        float FinalFlowStrength = FlowStrength;

        const FVector FlowForce = PushDir * FinalFlowStrength * Movement->GetMaxAcceleration() * 0.0167f;

        Movement->Velocity += FlowForce;
        LOGV(Log, TEXT("Velocity : %f"), Movement->Velocity.Length());
    }
}
