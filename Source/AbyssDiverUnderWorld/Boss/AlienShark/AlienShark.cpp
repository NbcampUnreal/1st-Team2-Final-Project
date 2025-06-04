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

bool AAlienShark::IsLocationOnNavMesh(const FVector& InLocation) const
{
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem)) return false;
    
    FNavLocation NavLocation;

    // 액터가 NavMesh 상에 존재하는지에 대한 여부를 반환한다.
    return NavSystem->ProjectPointToNavigation(InLocation, NavLocation, FVector(100.0f, 100.0f, 100.0f));
}

FVector AAlienShark::GetRandomNavMeshLocation(const FVector& Origin, const float& Radius) const
{
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem))
    {
        return Origin;
    }

    FNavLocation NavLocation;
    if (NavSystem->GetRandomReachablePointInRadius(Origin, Radius, NavLocation))
    {
        return NavLocation.Location;
    }
    
    return Origin;
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
        SetNewTarget();
        
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

void AAlienShark::SetNewTarget()
{
    const FVector CurrentLocation = GetActorLocation();
    
    // 현재 전방 방향을 기준으로 먼저 목표를 찾아보기 (자연스러운 이동을 위해)
    const FVector Forward = GetActorForwardVector();
    const FVector Right = GetActorRightVector();
    const FVector Up = GetActorUpVector();
    
    // 전방 위주의 각도 범위로 제한하여 급격한 방향 전환 방지
    for (uint8 Attempts = 0; Attempts < 15; Attempts++)
    {
        // 전방 위주로 각도 범위 제한 (-60도 ~ +60도)
        const float HorizontalAngle = FMath::RandRange(-60.0f, 60.0f);
        const float VerticalAngle = FMath::RandRange(-30.0f, 30.0f);

        // Horizontal 및 Vertical 각도를 적용하여 새로운 방향 벡터를 생성한다.
        FVector NewDirection = Forward.RotateAngleAxis(HorizontalAngle, Up);
        NewDirection = NewDirection.RotateAngleAxis(VerticalAngle, Right);
        NewDirection.Normalize();
        
        const float RandomDistance = FMath::RandRange(WanderRadius * 0.7f, WanderRadius * 1.3f);
        FVector PotentialTarget = CurrentLocation + NewDirection * RandomDistance;
        
        // 생성한 지점이 NavMesh 지점이 아니라면 새로 생성한다.
        if (!IsLocationOnNavMesh(PotentialTarget))
        {
            continue;
        }
        
        FHitResult HitResult;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CurrentLocation,
            PotentialTarget,
            ECC_Visibility,
            Params
        );
        
        if (!bHit)
        {
            TargetLocation = PotentialTarget;
            
            LOG(TEXT("Forward-biased target set (Attempt %d): %s (Distance: %f)"), 
                   Attempts + 1,
                   *TargetLocation.ToString(), 
                   FVector::Dist(CurrentLocation, TargetLocation));

#if WITH_EDITOR
            DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Yellow, false, 3.0f, 0, 5.0f);
            DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, FColor::Yellow, false, 3.0f, 0, 3.0f);
#endif
            
            return;
        }
    }
    
    // 전방 우선 탐색 실패시 NavMesh 기반 탐색
    const FVector NavMeshTarget = GetRandomNavMeshLocation(CurrentLocation, WanderRadius);

    // NavMesh 목표지점 할당에 성공한 경우 얼리 리턴한다.
    if (!(FVector::Dist(NavMeshTarget, CurrentLocation) <= KINDA_SMALL_NUMBER))
    {
        TargetLocation = NavMeshTarget;

        LOG(TEXT("NavMesh target set: %s (Distance: %f)"), 
               *TargetLocation.ToString(), 
               FVector::Dist(CurrentLocation, TargetLocation));
        
#if WITH_EDITOR
        // 디버그용 목표점 표시
        DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Cyan, false, 3.0f, 0, 5.0f);
        DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, FColor::Cyan, false, 3.0f, 0, 3.0f);
#endif
        
        return;
    }
    
    // 모든 시도가 실패하면 NavMesh 내에서 가까운 점으로 지정한다.
    // 만약 재설정에 실패한 경우 강제로 재설정 지점을 지정한다.
    TargetLocation = GetRandomNavMeshLocation(CurrentLocation, MinTargetDistance * 3.0f);
    if (FVector::Dist(TargetLocation, CurrentLocation) <= KINDA_SMALL_NUMBER)
    {
        TargetLocation = CurrentLocation + Forward * MinTargetDistance;
    }
    
    LOG(TEXT("Fallback target set: %s"), *TargetLocation.ToString());

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
#endif
}

void AAlienShark::PerformNormalMovement(float DeltaTime)
{
    const FVector CurrentLocation = GetActorLocation();
    
    // 목표점이 없거나 Nav Mesh를 벗어났으면 목표점을 새로 설정한다.
    if (TargetLocation.IsZero() || !IsLocationOnNavMesh(TargetLocation))
    {
        LOG(TEXT("Target invalid or outside NavMesh, setting new target"));
        SetNewTarget();
        return;
    }
    
    const float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    
    // 목표점에 도달했으면 새 목표점 설정
    if (DistanceToTarget < MinTargetDistance)
    {
        LOG(TEXT("Reached target (Distance: %f), setting new target"), DistanceToTarget);
        SetNewTarget();
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
        SetNewTarget();
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
        
        SetNewTarget();
        
        LOG(TEXT("Turn completed, setting new target"));
    }
}
