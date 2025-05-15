#include "CurrentZone.h"
#include "Character/UnderwaterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ACurrentZone::ACurrentZone()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
    RootComponent = TriggerZone;

    // 충돌 설정
    //TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //TriggerZone->SetCollisionObjectType(ECC_WorldStatic);
    //TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    //TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    //TriggerZone->SetGenerateOverlapEvents(true);

    // 오버랩 이벤트 바인딩
    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACurrentZone::OnOverlapBegin);
    TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ACurrentZone::OnOverlapEnd);
}

void ACurrentZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector FlowDirection = FVector(0.f, -1.f, 0.f); // 아래로 밀기

    DrawDebugLine(GetWorld(),
        GetActorLocation(),
        GetActorLocation() + FlowDirection * 100.0f,
        FColor::Red, false, 0.1f, 0, 3.f);

    const float Force = 100.f;
    const float MaxDownwardSpeed = -800.f; // 최대 낙하 속도 제한 (절댓값이 클수록 빠름)

    for (ACharacter* Char : AffectedCharacters)
    {
        if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(Char))
        {
            UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
            if (Movement)
            {
                // 현재 속도에 힘 추가
                Movement->Velocity += FlowDirection * Force * DeltaTime;

                // Z축 속도만 Clamp
                Movement->Velocity.Z = FMath::Clamp(Movement->Velocity.Z, MaxDownwardSpeed, 0.f);

                UE_LOG(LogTemp, Warning, TEXT("💨 이동 시도 중: %s | Z속도: %.2f"), *Character->GetName(), Movement->Velocity.Z);
            }
        }
    }
}



void ACurrentZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ 급류 진입: %s"), *Character->GetName());
        AffectedCharacters.Add(Character);

        // 속도 제한 해제
        Character->GetCharacterMovement()->MaxWalkSpeed = 3000.f;
        Character->GetCharacterMovement()->MaxAcceleration = 6000.f;

        // 즉시 강한 밀림 효과
        FVector LaunchVelocity = FVector(0.f, 1.f, 0.f) * 1500.f;
        Character->LaunchCharacter(LaunchVelocity, true, false);
    }
}

void ACurrentZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor))
    {
        AffectedCharacters.Remove(Character);

        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

        if (Movement)
        {

            Movement->StopMovementImmediately();

            Movement->MaxWalkSpeed = 600.f;
            Movement->MaxAcceleration = 2048.f;
        }

        UE_LOG(LogTemp, Warning, TEXT("⛔ 급류 종료: %s → 완전 정지 처리됨"), *Character->GetName());
    }
}

