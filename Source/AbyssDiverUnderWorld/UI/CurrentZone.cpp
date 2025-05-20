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
        AffectedCharacters.Add(Character);

        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (!OriginalSpeeds.Contains(Character))
            {
                OriginalSpeeds.Add(Character, Movement->MaxWalkSpeed);
                OriginalAccelerations.Add(Character, Movement->MaxAcceleration);
            }

            FVector LaunchVelocity = PushDirection.GetSafeNormal() * 1000.f;
            Character->LaunchCharacter(LaunchVelocity, true, false);

            // 무조건 타이머 재시작 (중복 등록 방지용 IsTimerActive 제거)
            GetWorldTimerManager().SetTimer(CurrentForceTimer, this, &ACurrentZone::ApplyCurrentForce, 0.05f, true);

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

        if (AffectedCharacters.Num() == 0)
        {
            GetWorldTimerManager().ClearTimer(CurrentForceTimer);
        }

        FTimerHandle RecoverMovementTimer;
        GetWorld()->GetTimerManager().SetTimer(RecoverMovementTimer, [Character]()
            {
                if (Character && Character->GetCharacterMovement())
                {
                    Character->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
                    UE_LOG(LogTemp, Log, TEXT("🏊‍♂️ 수영 모드 복구 [%s]"), *Character->GetName());
                }
            }, 2.0f, false);
    }
}

void ACurrentZone::ApplyCurrentForce()
{
    for (AUnderwaterCharacter* Character : AffectedCharacters)
    {
        if (!Character || !Character->GetCharacterMovement())
            continue;

        auto* Movement = Character->GetCharacterMovement();

        if (Movement->IsFalling() || Movement->MovementMode == MOVE_None)
        {
            Movement->SetMovementMode(MOVE_Swimming);
        }

        // ✅ 직접 Velocity를 계속 덧붙여서 밀어주는 방식
        FVector PushDir = PushDirection.GetSafeNormal();
        FVector CurrentVelocity = Movement->Velocity;

        // 상하좌우 입력은 그대로 유지되되, 흐름 방향으로 추가 속도 부여
        float FlowStrength = 50.f; // ← 조절 가능: 물살 세기
        FVector FlowForce = PushDir * FlowStrength * Movement->GetMaxAcceleration() * GetWorld()->DeltaTimeSeconds;

        Movement->Velocity += FlowForce;
    }
}
