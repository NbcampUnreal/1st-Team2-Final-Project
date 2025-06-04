#include "Boss/AlienShark/AlienShark.h"

#include "NavigationSystem.h"
#include "Character/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AAlienShark::AAlienShark()
{
	PrimaryActorTick.bCanEverTick = true;
	BiteCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Collision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteCollision->SetCapsuleHalfHeight(80.0f);
	BiteCollision->SetCapsuleRadius(80.0f);
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->ComponentTags.Add(TEXT("Bite Collision"));
}

void AAlienShark::BeginPlay()
{
	Super::BeginPlay();

	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
}

void AAlienShark::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // NavMeshCheckInterval 마다 현재 액터가 NavMesh 내에 있는지 확인한다.
    NavMeshCheckTimer += DeltaTime;
    if (NavMeshCheckTimer >= NavMeshCheckInterval)
    {
        NavMeshCheckTimer = 0.0f;
        if (!IsLocationOnNavMesh(GetActorLocation()))
        {
            ReturnToNavMeshArea();
            return;
        }
    }
	
    // 1. 회전 중이면 회전만 수행
    if (bIsTurning)
    {
        PerformTurn(DeltaTime);
    }
    // 2. 전방에 장애물이 있으면 회전 시작
    else if (HasObstacleAhead())
    {
        StartTurn();
    }
    // 3. 일반 이동
    else
    {
        PerformNormalMovement(DeltaTime);
    }
    
    // 표면 추적은 별도로 (회전에 영향 주지 않음)
    SmoothMoveAlongSurface(DeltaTime);
}
