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

	Params.AddIgnoredActor(this);
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

void AAlienShark::ReturnToNavMeshArea()
{
    LOG(TEXT("Outside NavMesh! Returning to valid area"));
    
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem)) return;
    
    const FVector CurrentLocation = GetActorLocation();
    FNavLocation ClosestNavLocation;

    // 현재 위치에서 가장 가까운 NavMesh 지점을 찾아 이동한다.
    // 이동에 성공한 경우 목표 지점을 초기화 한 후 재설정한다.
    if (NavSystem->ProjectPointToNavigation(CurrentLocation, ClosestNavLocation, FVector(1000.0f, 1000.0f, 1000.0f)))
    {
        SetActorLocation(ClosestNavLocation.Location, true);
        
        TargetLocation = FVector::ZeroVector;
        SetNewTargetLocation();
        
        LOG(TEXT("Returned to NavMesh at: %s"), *ClosestNavLocation.Location.ToString());
    }
}

bool AAlienShark::HasObstacleAhead()
{
    const FVector Start = GetActorLocation();
    const FVector End = Start + GetActorForwardVector() * TraceDistance;
    
    // 물리적 장애물 체크
    FHitResult HitResult;
    const bool bPhysicalHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );
    
    // 라인 트레이싱 하는 지점이 Nav Mesh 내에 존재하는지 확인한다.
    const bool bNavMeshBlocked = !IsLocationOnNavMesh(End);

#if WITH_EDITOR
    FColor DebugColor = FColor::Green;

    // 전방의 NavMesh에서 벗어난 곳에 장애물이 존재한다면 **RED**
    if (bPhysicalHit && bNavMeshBlocked)
    {
        DebugColor = FColor::Red;
    }
    // NavMesh 내에서 전방에 장애물이 존재한다면 **ORANGE**
    else if (bPhysicalHit)
    {
        DebugColor = FColor::Orange;
    }
    // NavMesh를 벗어난 곳을 라인 트레이싱하고 있는 경우 **PURPLE**
    else if (bNavMeshBlocked)
    {
        DebugColor = FColor::Purple;
    }
    
    DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.1f, 0, 3.0f);
#endif
    
    return bPhysicalHit || bNavMeshBlocked;;
}

// 회전 시작 (Nav Mesh 고려)
void AAlienShark::StartTurn()
{
    bIsTurning = true;
    TurnTimer = 0.0f;
    
    // 3D 공간에서 가능한 방향들 탐색
    const FVector Right = GetActorRightVector();
    const FVector Up = GetActorUpVector();
    const FVector Forward = GetActorForwardVector();
    
    TArray<FVector> PossibleDirections = {
        // 수평 방향들
        Forward.RotateAngleAxis(30.0f, Up),
        Forward.RotateAngleAxis(-30.0f, Up),
        Forward.RotateAngleAxis(60.0f, Up),
        Forward.RotateAngleAxis(-60.0f, Up),
        Forward.RotateAngleAxis(90.0f, Up),
        Forward.RotateAngleAxis(-90.0f, Up),
        
        // 수직 방향들
        Forward.RotateAngleAxis(45.0f, Right),
        Forward.RotateAngleAxis(-45.0f, Right),
        
        // 대각선 방향들
        Forward.RotateAngleAxis(45.0f, Up).RotateAngleAxis(30.0f, Right),
        Forward.RotateAngleAxis(-45.0f, Up).RotateAngleAxis(30.0f, Right),
        Forward.RotateAngleAxis(45.0f, Up).RotateAngleAxis(-30.0f, Right),
        Forward.RotateAngleAxis(-45.0f, Up).RotateAngleAxis(-30.0f, Right)
    };
    
    // 수평, 수직, 대각선 방향에 대해 라인 트레이싱을 한다.
    // NavMesh를 벗어나지 않았고, 충돌이 발생하지 않는 방향을 목표 회전 값으로 저장한다.
    for (const FVector& Direction : PossibleDirections)
    {
        const FVector Start = GetActorLocation();
        
        // 더 멀리까지 체크해서 지속적으로 이동 가능한 방향인지 확인
        bool bDirectionViable = true;
        for (float CheckDistance = TraceDistance; CheckDistance <= TraceDistance * 2.0f; CheckDistance += TraceDistance * 0.5f)
        {
            const FVector End = Start + Direction * CheckDistance;
            
            FHitResult HitResult;
            const bool bPhysicalHit = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                Start,
                End,
                ECC_Visibility,
                Params
            );
            
            const bool bNavMeshBlocked = !IsLocationOnNavMesh(End);
            
            if (bPhysicalHit || bNavMeshBlocked)
            {
                bDirectionViable = false;
                break;
            }
        }
        
        const FVector End = Start + Direction * TraceDistance;

#if WITH_EDITOR
        FColor DebugColor = bDirectionViable ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 1.0f, 0, 2.0f);
#endif
        
        if (bDirectionViable)
        {
            TurnDirection = Direction;
            LOG(TEXT("Turn direction found: %s"), *Direction.ToString());
            return;
        }
    }
    
    // 모든 방향이 막혔으면 Nav Mesh 내에서 랜덤한 방향으로 목표 회전 값을 추출한다.
    const FVector RandomNavMeshPoint = GetRandomNavMeshLocation(GetActorLocation(), WanderRadius);
    TurnDirection = (RandomNavMeshPoint - GetActorLocation()).GetSafeNormal();
    
    LOG(TEXT("All directions blocked, turning toward random NavMesh point"));
}


void AAlienShark::PerformNormalMovement(float DeltaTime)
{
    const FVector CurrentLocation = GetActorLocation();
    
    // 목표점이 없거나 Nav Mesh를 벗어났으면 목표점을 새로 설정한다.
    if (TargetLocation.IsZero() || !IsLocationOnNavMesh(TargetLocation))
    {
        LOG(TEXT("Target invalid or outside NavMesh, setting new target"));
        SetNewTargetLocation();
        return;
    }
    
    const float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    
    // 목표점에 도달했으면 새 목표점 설정
    if (DistanceToTarget < MinTargetDistance)
    {
        LOG(TEXT("Reached target (Distance: %f), setting new target"), DistanceToTarget);
        SetNewTargetLocation();
        return;
    }
    
    // 목표점의 방향 벡터를 구한다.
    const FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    const FRotator CurrentRotation = GetActorRotation();
    const FRotator TargetRotation = ToTarget.Rotation();
    
    // 목표 방향벡터와 액터의 전방벡터를 내적한다.
    // 내적한 값을 라디안 각도로 변환한다.
    const FVector CurrentForward = GetActorForwardVector();
    const float DotProduct = FVector::DotProduct(CurrentForward, ToTarget);
    const float AngleDifference = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
    
    // 목표점이 너무 뒤쪽에 있거나 접근하기 어려운 경우 새로 목표점을 지정한다.
    if (AngleDifference > 120.0f || DistanceToTarget > WanderRadius * 2.0f)
    {
        LOG(TEXT("Target too far or behind, setting new target"));
        SetNewTargetLocation();
        return;
    }
    
    // 다음 이동 위치가 Nav Mesh 내인지 미리 체크한다.
    // 전방벡터를 기준으로 이동하므로 전방벡터를 활용하여 NavMesh를 확인해도 된다.
    const FVector NextLocation = CurrentLocation + CurrentForward * StatComponent->MoveSpeed * DeltaTime;
    if (!IsLocationOnNavMesh(NextLocation))
    {
        StartTurn();
        return;
    }
    
    // 목표 회전 값이 90도보다 큰 경우 회전 속도를 절반으로 줄여 자연스러운 회전을 연출한다.
    float AdjustedRotationSpeed = RotationSpeed;
    if (AngleDifference > 90.0f)
    {
        AdjustedRotationSpeed *= 0.5f;
    }
    
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AdjustedRotationSpeed);
    SetActorRotation(NewRotation);
    
    // 각도차이에 따른 부드러운 속도 조절 (선형 보간 적용)
    constexpr float MinMultiplier = 0.6f;
    constexpr float MaxMultiplier = 1.0f;
    constexpr float MidAngle = 30.0f;
    constexpr float MaxAngle = 60.0f;

    float SpeedMultiplier = MaxMultiplier;
    if (AngleDifference > MidAngle)
    {
        // 30~60도 구간에서 1.0~0.6으로 선형 보간
        float T = FMath::Clamp((AngleDifference - MidAngle) / (MaxAngle - MidAngle), 0.0f, 1.0f);
        SpeedMultiplier = FMath::Lerp(MaxMultiplier, MinMultiplier, T);
    }

    const float AdjustedSpeed = StatComponent->MoveSpeed * SpeedMultiplier;
    const FVector NewLocation = CurrentLocation + GetActorForwardVector() * AdjustedSpeed * DeltaTime;
    SetActorLocation(NewLocation, true);
}

void AAlienShark::SmoothMoveAlongSurface(const float& InDeltaTime)
{
    bool bHit = false;
    FHitResult NearestHit;
    float NearestDistance = TNumericLimits<float>::Max();
    
    TArray<FVector> Directions = {
        -GetActorUpVector(),
        GetActorUpVector(),
        GetActorRightVector(),
        -GetActorRightVector()
    };
    
    for (const FVector& Dir : Directions)
    {
        FHitResult HitResult;
        const FVector Start = GetActorLocation();
        const FVector End = Start + Dir * FourDirectionTraceDistance;

        bool bCurrentHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility,
            Params
        );

        if (bCurrentHit)
        {
            const float Distance = (HitResult.ImpactPoint - Start).Size();
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestHit = HitResult;
                bHit = true;
            }
        }
#if WITH_EDITOR
        DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.1f, 0, 2.0f);
#endif
    }

    if (bHit)
    {
        // 상하좌우 라인트레이싱을 통해 Hit에 성공한 경우 Hit에 해당하는 **표면법선벡터**를 가져온다.
        // **표면법선벡터**와 **전방벡터**의 외적벡터를 구하면 **표면법선벡터**와 **전방벡터**에 수직인 벡터("A"라고 지칭)를 얻게된다.
        // A 벡터와 **표면법선벡터**를 다시 한 번 외적하게 되면 **표면법선벡터**에 평행한 벡터("B"라고 지칭)를 얻게된다.
        // 이후 B 벡터를 X축, 표면 법선벡터를 Z축으로 두는 회전 값을 추출한다.
        // 추출한 회전 값과 현재 회전 값을 보간하여 회전하도록 하면 부드럽게 벽에 붙어서 이동하는 느낌을 연출할 수 있다.
        const FVector SurfaceNormal = NearestHit.ImpactNormal;
        const FVector Right = FVector::CrossProduct(SurfaceNormal, GetActorForwardVector()).GetSafeNormal();
        const FVector AdjustedForward = FVector::CrossProduct(Right, SurfaceNormal).GetSafeNormal();
        const FRotator TargetRotation = FRotationMatrix::MakeFromXZ(AdjustedForward, SurfaceNormal).Rotator();
        const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRotation, InDeltaTime, RotationSpeed);
        SetActorRotation(NewRot);
    }
    else
    {
        // 상하좌우 라인트레이싱이 모두 Hit이 아닌 경우에는 Roll을 0으로 보간하여 회전시킨다.
        // Roll이 회전된 상태로 이동하면 부자연스럽기 때문에 자연스러움을 연출시키기 위함이다.
        const FRotator CurrentRot = GetActorRotation();
        FRotator TargetRot = CurrentRot;
        TargetRot.Roll = 0.0f;

        const FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRot, InDeltaTime, RotationSpeed);
        SetActorRotation(NewRotation);
    }
}

void AAlienShark::PerformTurn(const float& DeltaTime)
{
    TurnTimer += DeltaTime;
    
    // 목표 방향으로 회전
    const FRotator CurrentRotation = GetActorRotation();
    const FRotator TargetRotation = TurnDirection.Rotation();
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
    
    SetActorRotation(NewRotation);
    
    // 회전 중일 때에도 느린 이동속도로 이동한다. 자연스러운 연출을 위함이다.
    const FVector NewLocation = GetActorLocation() + GetActorForwardVector() * StatComponent->MoveSpeed * 0.4f * DeltaTime;
    SetActorLocation(NewLocation, true);
    
    // 목표 회전 값과 현재 회전 값의 차이가 15도 미만이거나
    // 회전을 시작한 지 2.0초를 초과했다면 회전을 종료하고 목표 지점을 재설정한다.
    const float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));
    if (AngleDifference < 15.0f || TurnTimer > 2.0f)
    {
        bIsTurning = false;
        TurnTimer = 0.0f;
        
        SetNewTargetLocation();
        
        LOG(TEXT("Turn completed, setting new target"));
    }
}
