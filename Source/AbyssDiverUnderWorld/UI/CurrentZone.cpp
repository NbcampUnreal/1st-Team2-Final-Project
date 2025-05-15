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

    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACurrentZone::OnOverlapBegin);
    TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ACurrentZone::OnOverlapEnd);
}

void ACurrentZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Add(Character);

        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            // 기본값 저장
            if (!OriginalSpeeds.Contains(Character))
            {
                OriginalSpeeds.Add(Character, Movement->MaxWalkSpeed);
                OriginalAccelerations.Add(Character, Movement->MaxAcceleration);
            }

            // 강제 속도 변경 대신 가볍게 Launch 처리
            FVector LaunchVelocity = PushDirection.GetSafeNormal() * 1000.f;
            Character->LaunchCharacter(LaunchVelocity, true, false);

            // 타이머 시작
            if (!GetWorldTimerManager().IsTimerActive(CurrentForceTimer))
            {
                GetWorldTimerManager().SetTimer(CurrentForceTimer, this, &ACurrentZone::ApplyCurrentForce, 0.05f, true);
            }

            FVector Velocity = Character->GetVelocity();
            float Speed = Velocity.Size();
            float DirDot = !Velocity.IsNearlyZero() ? FVector::DotProduct(Velocity.GetSafeNormal(), PushDirection.GetSafeNormal()) : 0.f;

            UE_LOG(LogTemp, Log, TEXT("✅ 급류 진입 [%s] | Speed: %.1f | DirDot: %.2f"),
                *Character->GetName(), Speed, DirDot);
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
            // 속도 정지
            Movement->StopMovementImmediately();

            // 기본값 복원
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

            FVector Velocity = Character->GetVelocity();
            float Speed = Velocity.Size();
            float DirDot = !Velocity.IsNearlyZero() ? FVector::DotProduct(Velocity.GetSafeNormal(), PushDirection.GetSafeNormal()) : 0.f;

            UE_LOG(LogTemp, Log, TEXT("⛔ 급류 탈출 [%s] | Speed: %.1f | DirDot: %.2f"),
                *Character->GetName(), Speed, DirDot);
        }

        // 타이머 정지
        if (AffectedCharacters.Num() == 0)
        {
            GetWorldTimerManager().ClearTimer(CurrentForceTimer);
        }
    }
}

void ACurrentZone::ApplyCurrentForce()
{
    for (AUnderwaterCharacter* Character : AffectedCharacters)
    {
        if (!Character || !Character->GetCharacterMovement()) continue;

        FVector Direction = PushDirection.GetSafeNormal();


        FVector InputVector = Character->GetLastMovementInputVector().GetSafeNormal();
        float Dot = FVector::DotProduct(InputVector, Direction);  

    
        if (Dot < -0.1f)
        {
            continue;
        }

        float ForceScale = (Dot < 0.2f) ? 0.2f : 0.35f; // 기존 0.1 / 0.2 → 증가
        Character->AddMovementInput(Direction, ForceScale);

    }
}

