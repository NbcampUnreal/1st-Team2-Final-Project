// Fill out your copyright notice in the Description page of Project Settings.

// AquaticMovementComponent.cpp

#include "AquaticMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogAquaticMovement);

UAquaticMovementComponent::UAquaticMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // 기본 본 설정
    TrackedBones.Add({ TEXT("spine_01"), 1.0f, 1.0f });
    TrackedBones.Add({ TEXT("spine_02"), 1.0f, 1.0f });
    TrackedBones.Add({ TEXT("spine_03"), 1.0f, 1.0f });
    TrackedBones.Add({ TEXT("tail_01"), 0.9f, 0.8f });
    TrackedBones.Add({ TEXT("tail_02"), 0.8f, 0.6f });
    TrackedBones.Add({ TEXT("tail_03"), 0.7f, 0.4f });
    
    AverageBoneDistance = 0.0f;
    bBoneDistanceCalculated = false;
    WallFollowingMode = 0;
    TargetDirectionWeight = 1.5f;
    CurrentWallFollowDirection = 0;
    LastWallDetectionTime = -1.0f;
    LastWallNormal = FVector::ZeroVector;
}

void UAquaticMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        InitComponent(OwnerCharacter);
    }

    LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    // 본 간 평균 거리 계산
    CalculateAverageBoneDistance();
}

void UAquaticMovementComponent::InitComponent(ACharacter* InCharacter)
{
    if (!InCharacter) return;

    OwnerCharacter = InCharacter;
    CharacterMesh = OwnerCharacter->GetMesh();

    if (!CharacterMesh)
    {
        UE_LOG(LogAquaticMovement, Error, TEXT("Character has no mesh!"));
        return;
    }

    // 초기 경로 포인트 예약
    TrajectoryHistory.Reserve(MaxTrajectoryPoints);

    // 초기 위치 기록
    RecordTrajectoryPoint(1.0f);
    LastRecordedLocation = OwnerCharacter->GetActorLocation();

    UE_LOG(LogAquaticMovement, Log, TEXT("Aquatic movement initialized with %d tracked bones"), TrackedBones.Num());
}

void UAquaticMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !CharacterMesh) return;

    // LOD 팩터 업데이트
    CurrentLODFactor = GetLODFactor();

    // 이동 업데이트
    UpdateMovement(DeltaTime);

    // 경로 업데이트
    UpdateTrajectory(DeltaTime);

    // 본 추적 업데이트
    UpdateBoneTrailsDistanceBased(DeltaTime);

    // 디버그 시각화
    if (bDrawTrajectory || bDrawBoneTrails || bDrawAvoidance)
    {
        DrawDebugVisualization();
    }
}

// === 이동 업데이트 ===
void UAquaticMovementComponent::UpdateMovement(float DeltaTime)
{
    // 목표 도달 확인
    if (bHasTarget)
    {
        float DistanceToTarget = (TargetLocation - OwnerCharacter->GetActorLocation()).Size();
        // AcceptanceRadius에 도달하고 속도가 충분히 낮으면 완료
        if (DistanceToTarget <= TargetAcceptanceRadius && CurrentVelocity.Size() < 50.0f)
        {
            // 목표에 도달 - 타겟 추적 중지
            bHasTarget = false;
            CurrentVelocity = FVector::ZeroVector;
            return;
        }
    }
    
    FVector SteeringForce = CalculateSteeringForce();
    FVector AvoidanceForce = CalculateAvoidanceForce();

    // 장애물 회피 로그
    if (!AvoidanceForce.IsZero())
    {
        UE_LOG(LogAquaticMovement, Warning, TEXT("Avoidance active: Force=%.1f, SteeringForce=%.1f"), 
            AvoidanceForce.Size(), SteeringForce.Size());
    }

    // 힘 합성 (장애물 회피 우선)
    FVector TotalForce = SteeringForce + AvoidanceForce * ObstacleAvoidanceStrength;

    // 가속도 적용
    FVector NewAcceleration = TotalForce; // /10(가상 질량)
    FVector NewDesiredVelocity = CurrentVelocity + NewAcceleration * DeltaTime;
    
    // 경로 곡률 제한 (속도 방향 변화 제한)
    if (!CurrentVelocity.IsNearlyZero() && !NewDesiredVelocity.IsNearlyZero())
    {
        FVector CurrentDir = CurrentVelocity.GetSafeNormal();
        FVector DesiredDir = NewDesiredVelocity.GetSafeNormal();
        
        // 현재 방향과 원하는 방향 사이의 각도
        float AngleDifference = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CurrentDir | DesiredDir, -1.0f, 1.0f)));
        float MaxDeltaAngle = MaxTurnAnglePerSecond * DeltaTime;
        
        if (AngleDifference > MaxDeltaAngle)
        {
            // 회전축 계산 (외적)
            FVector RotationAxis = CurrentDir ^ DesiredDir;
            if (RotationAxis.IsNearlyZero())
            {
                // 180도 회전의 경우 임의의 수직 축 사용
                RotationAxis = CurrentDir ^ FVector::UpVector;
                if (RotationAxis.IsNearlyZero())
                {
                    RotationAxis = CurrentDir ^ FVector::RightVector;
                }
            }
            RotationAxis.Normalize();
            
            // 제한된 각도만큼 회전
            FQuat Rotation = FQuat(RotationAxis, FMath::DegreesToRadians(MaxDeltaAngle));
            FVector NewDir = Rotation.RotateVector(CurrentDir);
            
            // 회전 반경을 만들기 위한 측면 이동 추가
            // 회전 방향에 수직인 방향으로 이동하여 부드러운 곡선 생성
            FVector SideDir = RotationAxis ^ CurrentDir;
            SideDir.Normalize();
            
            // 회전 속도에 비례하는 측면 이동량 계산
            float TurnRadius = CurrentVelocity.Size() / FMath::DegreesToRadians(MaxTurnAnglePerSecond);
            float SideSpeed = CurrentVelocity.Size() * FMath::Sin(FMath::DegreesToRadians(MaxDeltaAngle));
            
            // 새로운 속도 = 전진 성분 + 측면 이동 성분
            FVector ForwardComponent = NewDir * (CurrentVelocity.Size() * FMath::Cos(FMath::DegreesToRadians(MaxDeltaAngle)));
            FVector SideComponent = SideDir * SideSpeed;
            
            CurrentVelocity = ForwardComponent + SideComponent;
            
            // 속도 크기 유지
            if (!CurrentVelocity.IsNearlyZero())
            {
                CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewDesiredVelocity.Size();
            }
        }
        else
        {
            CurrentVelocity = NewDesiredVelocity;
        }
    }
    else
    {
        CurrentVelocity = NewDesiredVelocity;
    }
    
    // 속도 제한
    if (CurrentVelocity.Size() > MaxSpeed)
    {
        CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
    }

    // 감속 처리
    if (TotalForce.IsNearlyZero() && !CurrentVelocity.IsNearlyZero())
    {
        float DecelerationAmount = BrakingDeceleration * DeltaTime;
        if (CurrentVelocity.Size() <= DecelerationAmount)
        {
            CurrentVelocity = FVector::ZeroVector;
        }
        else
        {
            CurrentVelocity -= CurrentVelocity.GetSafeNormal() * DecelerationAmount;
        }
    }

    // 위치 업데이트
    if (!CurrentVelocity.IsNearlyZero())
    {
        FVector NewLocation = OwnerCharacter->GetActorLocation() + CurrentVelocity * DeltaTime;

        // 이동 전 충돌 예측 (벽면 타기 중일 때만)
        if (CurrentWallFollowDirection != 0)
        {
            FHitResult PredictHit;
            FVector CurrentLocation = OwnerCharacter->GetActorLocation();
            
            // 이동 방향으로 미리 체크
            if (GetWorld()->LineTraceSingleByChannel(PredictHit, CurrentLocation, NewLocation + CurrentVelocity.GetSafeNormal() * 50.0f, ECC_WorldStatic))
            {
                // 벽을 뚫으려고 하면 위치 조정
                FVector AdjustedLocation = PredictHit.Location - CurrentVelocity.GetSafeNormal() * 30.0f;
                NewLocation = AdjustedLocation;
                
                // 벽면 정보 업데이트
                LastWallNormal = PredictHit.Normal;
                LastWallDetectionTime = GetWorld()->GetTimeSeconds();
            }
        }

        // 회전 업데이트 - 경로 방향을 따라감
        FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
        FRotator TargetRotation = CurrentVelocity.GetSafeNormal().Rotation();
        
        // 부드러운 보간 적용 (경로가 이미 곡률 제한됨)
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnSpeed);

        OwnerCharacter->SetActorLocationAndRotation(NewLocation, NewRotation);
    }
}

FVector UAquaticMovementComponent::CalculateSteeringForce() const
{
    if (!bHasTarget) return FVector::ZeroVector;

    FVector ToTarget = TargetLocation - OwnerCharacter->GetActorLocation();
    float Distance = ToTarget.Size();

    // AcceptanceRadius에 도달했으면 정지
    if (Distance <= TargetAcceptanceRadius)
    {
        // 목표 도달 - 정지를 위한 브레이킹
        return -CurrentVelocity.GetSafeNormal() * BrakingDeceleration;
    }
    
    // 감속 거리 계산 (속도에 따라 동적으로)
    float StoppingDistance = (CurrentVelocity.Size() * CurrentVelocity.Size()) / (2.0f * BrakingDeceleration);
    float SlowdownDistance = StoppingDistance + TargetAcceptanceRadius;
    
    if (Distance < SlowdownDistance)
    {
        // 감속 구간: AcceptanceRadius에 정확히 멈출 수 있도록 감속
        float DistanceToSlowdown = Distance - TargetAcceptanceRadius;
        float TargetSpeed = MaxSpeed * (DistanceToSlowdown / (SlowdownDistance - TargetAcceptanceRadius));
        TargetSpeed = FMath::Max(0.0f, TargetSpeed);
        
        FVector NewDesiredVelocity = ToTarget.GetSafeNormal() * TargetSpeed;
        return NewDesiredVelocity - CurrentVelocity;
    }
    else
    {
        // 최대 속도로 이동
        FVector NewDesiredVelocity = ToTarget.GetSafeNormal() * MaxSpeed;
        return NewDesiredVelocity - CurrentVelocity;
    }
}

FVector UAquaticMovementComponent::CalculateAvoidanceForce() 
{
    FVector AvoidanceForce = FVector::ZeroVector;
    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    
    // 목표 방향 (있을 경우)
    FVector ToTarget = bHasTarget ? (TargetLocation - CurrentLocation).GetSafeNormal() : Forward;
    
    // 벽면 정보 추적
    FVector ClosestWallNormal = FVector::ZeroVector;
    float ClosestWallDistance = ObstacleDetectionRange;
    bool bFoundWallInPath = false;
    
    // 좌/우 측면 장애물 거리
    float LeftDistance = ObstacleDetectionRange;
    float RightDistance = ObstacleDetectionRange;
    
    // 벽면 타기 중이면 측면 감지 강화
    bool bCurrentlyWallFollowing = CurrentWallFollowDirection != 0 && 
                                   (GetWorld()->GetTimeSeconds() - LastWallDetectionTime) < 0.5f;

    // 레이캐스트 기반 장애물 감지
    for (int32 i = 0; i < AvoidanceRayCount; i++)
    {
        float Angle = (360.0f / AvoidanceRayCount) * i;
        FVector RayDirection = Forward.RotateAngleAxis(Angle, FVector::UpVector);

        FHitResult Hit;
        FVector RayStart = CurrentLocation;
        FVector RayEnd = RayStart + RayDirection * ObstacleDetectionRange;

        if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic))
        {
            float Distance = Hit.Distance;
            float BaseAvoidanceStrength = 1.0f - (Distance / ObstacleDetectionRange);
            
            // 목표 방향과의 각도 계산
            float DotToTarget = FVector::DotProduct(RayDirection, ToTarget);
            float DirectionWeight = 1.0f;
            
            // 벽면 타기 중이면 측면 장애물도 중요하게 처리
            if (bCurrentlyWallFollowing)
            {
                // 현재 따라가는 방향의 측면 체크
                FVector RightDir = Forward.Cross(FVector::UpVector);
                float SideDot = FVector::DotProduct(RayDirection, RightDir);
                
                // 좌수법이면 좌측, 우수법이면 우측 벽면 감지 강화
                bool bIsFollowingSide = (CurrentWallFollowDirection > 0 && SideDot < -0.5f) || 
                                       (CurrentWallFollowDirection < 0 && SideDot > 0.5f);
                
                if (bIsFollowingSide && Distance < ObstacleDetectionRange * 0.5f)
                {
                    DirectionWeight = TargetDirectionWeight * 0.8f; // 측면도 중요하게
                    bFoundWallInPath = true;
                    
                    if (Distance < ClosestWallDistance)
                    {
                        ClosestWallDistance = Distance;
                        ClosestWallNormal = Hit.Normal;
                    }
                }
            }
            
            // 목표 방향의 장애물에 더 큰 가중치
            if (DotToTarget > 0.5f) // 전방 90도 이내
            {
                DirectionWeight = TargetDirectionWeight;
                bFoundWallInPath = true;
                
                // 가장 가까운 벽면 정보 업데이트
                if (Distance < ClosestWallDistance)
                {
                    ClosestWallDistance = Distance;
                    ClosestWallNormal = Hit.Normal;
                }
            }
            
            // 좌/우 거리 측정
            FVector RightDir = Forward.Cross(FVector::UpVector);
            float SideDot = FVector::DotProduct(RayDirection, RightDir);
            if (SideDot > 0.7f) // 우측
            {
                RightDistance = FMath::Min(RightDistance, Distance);
            }
            else if (SideDot < -0.7f) // 좌측
            {
                LeftDistance = FMath::Min(LeftDistance, Distance);
            }
            
            // 벽면을 따라가는 경우 (벽면 타기 중이면 거리 조건 완화)
            float WallFollowThreshold = bCurrentlyWallFollowing ? 0.5f : 0.3f;
            if (bFoundWallInPath && ClosestWallDistance < ObstacleDetectionRange * WallFollowThreshold)
            {
                // 벽면의 접선 방향 계산
                FVector WallTangent;
                
                // 벽면이 변경되었는지 확인 (법선 방향 변화로 판단)
                float WallNormalDot = FVector::DotProduct(ClosestWallNormal, LastWallNormal);
                bool bWallChanged = WallNormalDot < 0.7f || (GetWorld()->GetTimeSeconds() - LastWallDetectionTime) > 1.0f;
                
                // 좌수법/우수법 선택
                int32 FollowDirection = WallFollowingMode;
                
                if (FollowDirection == 0) // 자동 선택
                {
                    // 새로운 벽을 만났거나 오랜만에 벽을 만난 경우에만 방향 재계산
                    if (bWallChanged || CurrentWallFollowDirection == 0)
                    {
                        // 목표 방향과 더 일치하는 방향 선택
                        FVector LeftTangent = ClosestWallNormal.Cross(FVector::UpVector);
                        FVector RightTangent = -LeftTangent;
                        
                        float LeftDot = FVector::DotProduct(LeftTangent, ToTarget);
                        float RightDot = FVector::DotProduct(RightTangent, ToTarget);
                        
                        CurrentWallFollowDirection = (LeftDot > RightDot) ? 1 : -1;
                    }
                    FollowDirection = CurrentWallFollowDirection;
                }
                
                if (FollowDirection > 0) // 좌수법
                {
                    WallTangent = ClosestWallNormal.Cross(FVector::UpVector);
                }
                else // 우수법
                {
                    WallTangent = FVector::UpVector.Cross(ClosestWallNormal);
                }
                
                // 벽면을 따라가는 힘
                AvoidanceForce += WallTangent * BaseAvoidanceStrength * DirectionWeight;
                
                // 벽에서 약간 떨어지는 힘
                AvoidanceForce += ClosestWallNormal * BaseAvoidanceStrength * 0.5f;
                
                // 벽면 정보 업데이트
                LastWallNormal = ClosestWallNormal;
                LastWallDetectionTime = GetWorld()->GetTimeSeconds();
            }
            else
            {
                // 일반적인 회피 (장애물로부터 멀어지기)
                FVector AwayFromObstacle = (CurrentLocation - Hit.Location).GetSafeNormal();
                AvoidanceForce += AwayFromObstacle * BaseAvoidanceStrength * DirectionWeight;
            }
        }
    }
    
    // 벽면에서 멀어진 경우 방향 초기화
    if (!bFoundWallInPath && (GetWorld()->GetTimeSeconds() - LastWallDetectionTime) > 2.0f)
    {
        CurrentWallFollowDirection = 0;
    }

    // 수동 추가된 회피 위치
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (const FObstacleInfo& Obstacle : ActiveObstacles)
    {
        // 오래된 장애물 무시
        if (CurrentTime - Obstacle.DetectionTime > 5.0f) continue;

        FVector ToObstacle = Obstacle.Location - CurrentLocation;
        float Distance = ToObstacle.Size();

        if (Distance < Obstacle.Radius * 2.0f)
        {
            float AvoidanceStrength = 1.0f - (Distance / (Obstacle.Radius * 2.0f));
            FVector AwayFromObstacle = -ToObstacle.GetSafeNormal();
            AvoidanceForce += AwayFromObstacle * AvoidanceStrength * Obstacle.AvoidanceStrength;
        }
    }

    return AvoidanceForce;
}

// === 경로 관리 ===
void UAquaticMovementComponent::UpdateTrajectory(float DeltaTime)
{
    // LOD 기반 기록 간격
    float RecordInterval = GetTrajectoryRecordInterval();

    if (GetWorld()->GetTimeSeconds() - LastRecordTime >= RecordInterval)
    {
        // 이동 거리 기반 중요도 계산
        float MovementDelta = (OwnerCharacter->GetActorLocation() - LastRecordedLocation).Size();
        float Importance = FMath::Clamp(MovementDelta / (MaxSpeed * RecordInterval), 0.1f, 1.0f);

        for (FTrajectoryPoint Point : TrajectoryHistory)
        {
            Point.Importance *= 0.9f;
        }
        RecordTrajectoryPoint(Importance);
        LastRecordTime = GetWorld()->GetTimeSeconds();
        LastRecordedLocation = OwnerCharacter->GetActorLocation();
    }

    // 오래된 포인트 정리
    CleanOldTrajectoryPoints();

    // LOD 기반 경로 최적화
    if (CurrentLODFactor < 0.7f)
    {
        //OptimizeTrajectoryForLOD(); //안정성에 비해 효과 크지 않아 제거함
    }

    // 미래 경로 예측 및 추가
    PredictFuturePath(TrajectoryPredictionTime);
}


void UAquaticMovementComponent::PredictFuturePath(float PredictionTime)
{
    if (CurrentVelocity.IsNearlyZero() && !bHasTarget)
    {
        return; // 움직이지 않고 목표도 없으면 예측할 필요 없음
    }

    // 예측에 사용할 변수들
    FVector PredictedLocation = OwnerCharacter->GetActorLocation();
    FRotator PredictedRotation = OwnerCharacter->GetActorRotation();
    FVector PredictedVelocity = CurrentVelocity;

    float TimeStep = 0.1f; // 100ms 간격으로 예측
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 기존 미래 예측 포인트 제거 (현재 시간 이후의 포인트들)
    TrajectoryHistory.RemoveAll([CurrentTime](const FTrajectoryPoint& Point)
        {
            return Point.Time > CurrentTime;
        });

    // 임시 예측 포인트 저장
    TArray<FTrajectoryPoint> PredictedPoints;

    for (float PredTime = TimeStep; PredTime <= PredictionTime; PredTime += TimeStep)
    {
        // 스티어링 힘 예측
        FVector SteeringForce = FVector::ZeroVector;

        if (bHasTarget)
        {
            // 목표를 향한 스티어링
            FVector ToTarget = TargetLocation - PredictedLocation;
            float Distance = ToTarget.Size();

            if (Distance > TargetAcceptanceRadius)
            {
                // 감속 거리 계산
                float PredictedStoppingDistance = (PredictedVelocity.Size() * PredictedVelocity.Size()) / (2.0f * BrakingDeceleration);
                float SlowdownDistance = PredictedStoppingDistance + TargetAcceptanceRadius;
                
                if (Distance < SlowdownDistance)
                {
                    // 감속 필요
                    float DistanceToSlowdown = Distance - TargetAcceptanceRadius;
                    float TargetSpeed = MaxSpeed * (DistanceToSlowdown / (SlowdownDistance - TargetAcceptanceRadius));
                    FVector DesiredVel = ToTarget.GetSafeNormal() * TargetSpeed;
                    SteeringForce = (DesiredVel - PredictedVelocity).GetClampedToMaxSize(Acceleration);
                }
                else
                {
                    // 최대 속도
                    FVector DesiredVel = ToTarget.GetSafeNormal() * MaxSpeed;
                    SteeringForce = (DesiredVel - PredictedVelocity).GetClampedToMaxSize(Acceleration);
                }
            }
            else
            {
                // 도달 - 정지
                SteeringForce = -PredictedVelocity.GetSafeNormal() * BrakingDeceleration;
            }
        }

        // 속도 업데이트 
        FVector NewPredictedVelocity = PredictedVelocity + SteeringForce * TimeStep;
        
        // 경로 곡률 제한
        if (!PredictedVelocity.IsNearlyZero() && !NewPredictedVelocity.IsNearlyZero())
        {
            FVector CurrentDir = PredictedVelocity.GetSafeNormal();
            FVector DesiredDir = NewPredictedVelocity.GetSafeNormal();
            
            float AngleDifference = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CurrentDir | DesiredDir, -1.0f, 1.0f)));
            float MaxDeltaAngle = MaxTurnAnglePerSecond * TimeStep;
            
            if (AngleDifference > MaxDeltaAngle)
            {
                FVector RotationAxis = CurrentDir ^ DesiredDir;
                if (RotationAxis.IsNearlyZero())
                {
                    RotationAxis = CurrentDir ^ FVector::UpVector;
                    if (RotationAxis.IsNearlyZero())
                    {
                        RotationAxis = CurrentDir ^ FVector::RightVector;
                    }
                }
                RotationAxis.Normalize();
                
                FQuat Rotation = FQuat(RotationAxis, FMath::DegreesToRadians(MaxDeltaAngle));
                FVector NewDir = Rotation.RotateVector(CurrentDir);
                
                // 회전 반경을 만들기 위한 측면 이동 추가
                FVector SideDir = RotationAxis ^ CurrentDir;
                SideDir.Normalize();
                
                float SideSpeed = PredictedVelocity.Size() * FMath::Sin(FMath::DegreesToRadians(MaxDeltaAngle));
                
                // 새로운 속도 = 전진 성분 + 측면 이동 성분
                FVector ForwardComponent = NewDir * (PredictedVelocity.Size() * FMath::Cos(FMath::DegreesToRadians(MaxDeltaAngle)));
                FVector SideComponent = SideDir * SideSpeed;
                
                PredictedVelocity = ForwardComponent + SideComponent;
                
                // 속도 크기 유지
                if (!PredictedVelocity.IsNearlyZero())
                {
                    PredictedVelocity = PredictedVelocity.GetSafeNormal() * NewPredictedVelocity.Size();
                }
            }
            else
            {
                PredictedVelocity = NewPredictedVelocity;
            }
        }
        else
        {
            PredictedVelocity = NewPredictedVelocity;
        }

        // 속도 제한
        if (PredictedVelocity.Size() > MaxSpeed)
        {
            PredictedVelocity = PredictedVelocity.GetSafeNormal() * MaxSpeed;
        }

        // 위치 업데이트
        PredictedLocation += PredictedVelocity * TimeStep;

        // 회전 업데이트 - 경로 방향을 따라감
        if (!PredictedVelocity.IsNearlyZero())
        {
            FRotator TargetRotation = PredictedVelocity.GetSafeNormal().Rotation();
            PredictedRotation = FMath::RInterpTo(PredictedRotation, TargetRotation, TimeStep, TurnSpeed);
        }

        // 예측 포인트 생성
        FTrajectoryPoint PredPoint;
        PredPoint.Location = PredictedLocation;
        PredPoint.Rotation = PredictedRotation;
        PredPoint.Time = CurrentTime + PredTime;
        PredPoint.Importance = 1.0f; 

        PredictedPoints.Add(PredPoint);
    }

    // 예측 포인트를 경로에 추가
    TrajectoryHistory.Append(PredictedPoints);
}

void UAquaticMovementComponent::RecordTrajectoryPoint(float Importance)
{
    FTrajectoryPoint NewPoint;
    NewPoint.Location = OwnerCharacter->GetActorLocation();
    NewPoint.Rotation = OwnerCharacter->GetActorRotation();
    NewPoint.Time = GetWorld()->GetTimeSeconds();
    NewPoint.Importance = Importance;
    
    // 크기 제한 체크
     if (TrajectoryHistory.Num() >= MaxTrajectoryPoints)
     {
         // 가장 중요도가 낮은 중간 포인트 제거
         //int32 RemoveIndex = 1; // 첫 번째는 유지
         //float LowestImportance = 1.0f;
     
         //for (int32 i = 1; i < TrajectoryHistory.Num() - 1; i++)
         //{
         //    if (TrajectoryHistory[i].Importance < LowestImportance)
         //    {
         //        LowestImportance = TrajectoryHistory[i].Importance;
         //        RemoveIndex = i;
         //    }
         //}
     
         TrajectoryHistory.RemoveAt(TrajectoryHistory.Num() - 1);
     }
    
    TrajectoryHistory.Add(NewPoint);
}

void UAquaticMovementComponent::OptimizeTrajectoryForLOD()
{
    if (TrajectoryHistory.Num() < 10) return;

    // LOD가 낮을수록 더 많은 포인트 제거
    float KeepRatio = FMath::Lerp(0.3f, 1.0f, CurrentLODFactor);
    int32 TargetCount = FMath::Max(10, (int32)(TrajectoryHistory.Num() * KeepRatio));

    while (TrajectoryHistory.Num() > TargetCount)
    {
        // 중요도가 가장 낮은 포인트 찾아서 제거
        int32 RemoveIndex = 1;
        float LowestImportance = 1.0f;

        for (int32 i = 1; i < TrajectoryHistory.Num() - 1; i++)
        {
            if (TrajectoryHistory[i].Importance < LowestImportance)
            {
                LowestImportance = TrajectoryHistory[i].Importance;
                RemoveIndex = i;
            }
        }

        TrajectoryHistory.RemoveAt(RemoveIndex);
    }
}

void UAquaticMovementComponent::CleanOldTrajectoryPoints()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    TrajectoryHistory.RemoveAll([&](const FTrajectoryPoint& Point)
        {
            return (CurrentTime - Point.Time) > TrajectoryMaxHistory;
        });
}

// InterpolateTrajectory 수정 - 음수 시간 지원 (미래 예측)
void UAquaticMovementComponent::InterpolateTrajectory(float TimeDelay, FVector& OutPosition, FRotator& OutRotation) const
{
    if (TrajectoryHistory.Num() < 2)
    {
        OutPosition = OwnerCharacter->GetActorLocation();
        OutRotation = OwnerCharacter->GetActorRotation();
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TargetTime = CurrentTime - TimeDelay; // 음수면 미래, 양수면 과거

    // 가장 가까운 두 포인트 찾기
    int32 Index1 = INDEX_NONE;
    int32 Index2 = INDEX_NONE;

    for (int32 i = 0; i < TrajectoryHistory.Num() - 1; i++)
    {
        if (TrajectoryHistory[i].Time <= TargetTime && TrajectoryHistory[i + 1].Time >= TargetTime)
        {
            Index1 = i;
            Index2 = i + 1;
            break;
        }
    }

    // 범위를 벗어난 경우 처리
    if (Index1 == INDEX_NONE)
    {
        if (TargetTime < TrajectoryHistory[0].Time)
        {
            // 가장 오래된 시점보다 이전
            OutPosition = TrajectoryHistory[0].Location;
            OutRotation = TrajectoryHistory[0].Rotation;
        }
        else
        {
            // 가장 최근 시점보다 미래 (예측된 경로 사용)
            OutPosition = TrajectoryHistory.Last().Location;
            OutRotation = TrajectoryHistory.Last().Rotation;
        }
        return;
    }

    // 근거리는 스플라인, 원거리는 선형 보간
    bool bUseSpline =  FMath::Abs(TimeDelay) < SplineInterpolationDistance&& TrajectoryHistory.Num() >= 4;

    if (bUseSpline && Index1 > 0 && Index2 < TrajectoryHistory.Num() - 1)
    {
        // Catmull-Rom 스플라인 보간
        const FTrajectoryPoint& P0 = TrajectoryHistory[FMath::Max(0, Index1 - 1)];
        const FTrajectoryPoint& P1 = TrajectoryHistory[Index1];
        const FTrajectoryPoint& P2 = TrajectoryHistory[Index2];
        const FTrajectoryPoint& P3 = TrajectoryHistory[FMath::Min(TrajectoryHistory.Num() - 1, Index2 + 1)];

        float T = (TargetTime - P1.Time) / (P2.Time - P1.Time);
        T = FMath::Clamp(T, 0.0f, 1.0f);

        OutPosition = CatmullRomSpline(P0.Location, P1.Location, P2.Location, P3.Location, T);
        OutRotation = FMath::Lerp(P1.Rotation, P2.Rotation, T);
    }
    else
    {
        // 선형 보간
        const FTrajectoryPoint& P1 = TrajectoryHistory[Index1];
        const FTrajectoryPoint& P2 = TrajectoryHistory[Index2];

        float Alpha = (TargetTime - P1.Time) / (P2.Time - P1.Time);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

        OutPosition = FMath::Lerp(P1.Location, P2.Location, Alpha);
        OutRotation = FMath::Lerp(P1.Rotation, P2.Rotation, Alpha);
    }
}

FVector UAquaticMovementComponent::CatmullRomSpline(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T) const
{
    float T2 = T * T;
    float T3 = T2 * T;

    return 0.5f * (
        (2.0f * P1) +
        (-P0 + P2) * T +
        (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 +
        (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3
        );
}

// === 본 추적 ===

void UAquaticMovementComponent::GetBoneDeltas(TArray<FVector>& OutPositionDeltas, TArray<FRotator>& OutRotationDeltas) const
{
    OutPositionDeltas.Empty(TrackedBones.Num());
    OutRotationDeltas.Empty(TrackedBones.Num());

    for (const FBoneTrailInfo& BoneInfo : TrackedBones)
    {
        OutPositionDeltas.Add(BoneInfo.PositionDelta);
        OutRotationDeltas.Add(BoneInfo.RotationDelta);
    }
}


// === 공개 함수들 ===
void UAquaticMovementComponent::SetTargetLocation(const FVector& Target, float AcceptanceRadius)
{
    TargetLocation = Target;
    TargetAcceptanceRadius = AcceptanceRadius;
    bHasTarget = true;
}

void UAquaticMovementComponent::AddAvoidanceLocation(const FVector& Location, float Radius, float Strength)
{
    FObstacleInfo NewObstacle;
    NewObstacle.Location = Location;
    NewObstacle.Radius = Radius;
    NewObstacle.AvoidanceStrength = Strength;
    NewObstacle.DetectionTime = GetWorld()->GetTimeSeconds();

    ActiveObstacles.Add(NewObstacle);

    // 오래된 장애물 정리
    float CurrentTime = GetWorld()->GetTimeSeconds();
    ActiveObstacles.RemoveAll([CurrentTime](const FObstacleInfo& Obstacle)
        {
            return CurrentTime - Obstacle.DetectionTime > 10.0f;
        });
}

void UAquaticMovementComponent::GetAllBoneOffsets(TArray<FVector>& OutOffsets, TArray<FRotator>& OutRotations) const
{
    OutOffsets.Empty(TrackedBones.Num());
    OutRotations.Empty(TrackedBones.Num());

    for (const FBoneTrailInfo& BoneInfo : TrackedBones)
    {
        //OutOffsets.Add(BoneInfo.CurrentOffset);
        //OutRotations.Add(BoneInfo.CurrentRotationOffset);
    }
}

float UAquaticMovementComponent::GetLODFactor() const
{
    if (!LocalPlayerController) return 1.0f;

    FVector PlayerLocation = LocalPlayerController->GetPawn() ? LocalPlayerController->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    float Distance = (OwnerCharacter->GetActorLocation() - PlayerLocation).Size();

    return FMath::Clamp(1.0f - (Distance - LODDistanceMin) / (LODDistanceMax - LODDistanceMin), 0.0f, 1.0f);
}

float UAquaticMovementComponent::GetTrajectoryRecordInterval() const
{
    return FMath::Lerp(LODMinRecordInterval, BaseRecordInterval, CurrentLODFactor);
}

// === 이동 제어 함수들 ===
void UAquaticMovementComponent::SetDesiredVelocity(const FVector& DesiredVel)
{
    DesiredVelocity = DesiredVel;
}

void UAquaticMovementComponent::StopMovement()
{
    DesiredVelocity = FVector::ZeroVector;
    bHasTarget = false;
    TargetLocation = FVector::ZeroVector;

    // 부드러운 정지를 위해 현재 속도는 감속을 통해 줄어들도록 함
}

void UAquaticMovementComponent::ClearAvoidanceLocations()
{
    ActiveObstacles.Empty();
}

float UAquaticMovementComponent::GetDistanceToTarget() const
{
    if (!bHasTarget || !OwnerCharacter) return 0.0f;
    
    return (TargetLocation - OwnerCharacter->GetActorLocation()).Size();
}

void UAquaticMovementComponent::CalculateAverageBoneDistance()
{
    if (!CharacterMesh || TrackedBones.Num() < 2)
    {
        AverageBoneDistance = 100.0f; // 기본값
        return;
    }
    
    float TotalDistance = 0.0f;
    int32 ValidPairs = 0;
    
    // 연속된 본들 간의 거리 계산
    for (int32 i = 0; i < TrackedBones.Num() - 1; ++i)
    {
        FName BoneName1 = TrackedBones[i].BoneName;
        FName BoneName2 = TrackedBones[i + 1].BoneName;
        
        int32 BoneIndex1 = CharacterMesh->GetBoneIndex(BoneName1);
        int32 BoneIndex2 = CharacterMesh->GetBoneIndex(BoneName2);
        
        if (BoneIndex1 != INDEX_NONE && BoneIndex2 != INDEX_NONE)
        {
            FTransform BoneTransform1 = CharacterMesh->GetBoneTransform(BoneIndex1);
            FTransform BoneTransform2 = CharacterMesh->GetBoneTransform(BoneIndex2);
            
            float Distance = (BoneTransform2.GetLocation() - BoneTransform1.GetLocation()).Size();
            TotalDistance += Distance;
            ValidPairs++;
        }
    }
    
    if (ValidPairs > 0)
    {
        AverageBoneDistance = TotalDistance / ValidPairs;
    }
    else
    {
        AverageBoneDistance = 100.0f; // 기본값
    }
    
    
    bBoneDistanceCalculated = true;
    
    UE_LOG(LogAquaticMovement, Log, TEXT("Average bone distance calculated: %f"), AverageBoneDistance);
}

// === 거리 기반 시스템 ===
void UAquaticMovementComponent::GetPositionAndRotationAlongPath(float DistanceFromHead, FVector& OutPosition, FRotator& OutRotation) const
{
    if (TrajectoryHistory.Num() < 2)
    {
        OutPosition = OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
        OutRotation = OwnerCharacter ? OwnerCharacter->GetActorRotation() : FRotator::ZeroRotator;
        return;
    }
    
    // 현재 시간을 기준으로 과거 포인트만 사용
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<FTrajectoryPoint> ValidPoints;
    
    for (const FTrajectoryPoint& Point : TrajectoryHistory)
    {
        if (Point.Time <= CurrentTime)
        {
            ValidPoints.Add(Point);
        }
    }
    
    if (ValidPoints.Num() < 2)
    {
        OutPosition = OwnerCharacter->GetActorLocation();
        OutRotation = OwnerCharacter->GetActorRotation();
        return;
    }
    
    // 시간순으로 정렬 (최신이 마지막)
    ValidPoints.Sort([](const FTrajectoryPoint& A, const FTrajectoryPoint& B)
    {
        return A.Time < B.Time;
    });
    
    // 현재 위치(머리)부터 거리를 따라 역추적
    float AccumulatedDistance = 0.0f;
    FVector CurrentPos = OwnerCharacter->GetActorLocation();
    
    // 가장 최근 점부터 역순으로 탐색
    for (int32 i = ValidPoints.Num() - 1; i >= 1; --i)
    {
        FVector NextPos = ValidPoints[i - 1].Location;
        float SegmentLength = (NextPos - CurrentPos).Size();
        
        if (AccumulatedDistance + SegmentLength >= DistanceFromHead)
        {
            // 이 세그먼트 내에서 목표 거리에 도달
            float RemainingDistance = DistanceFromHead - AccumulatedDistance;
            float Alpha = RemainingDistance / SegmentLength;
            
            OutPosition = FMath::Lerp(CurrentPos, NextPos, Alpha);
            
            // 다음 포인트를 바라보는 방향 계산
            if (i > 1) // 다음 포인트가 있으면
            {
                FVector ToNextPoint = ValidPoints[i - 2].Location - OutPosition;
                OutRotation = ToNextPoint.Rotation();
            }
            else
            {
                // 마지막 구간이면 현재 방향 유지
                OutRotation = (ValidPoints[i - 1].Location - OutPosition).Rotation();
            }
            
            return;
        }
        
        AccumulatedDistance += SegmentLength;
        CurrentPos = NextPos;
    }
    
    // 궤적이 충분히 길지 않으면 마지막 세그먼트 방향으로 연장
    if (AccumulatedDistance < DistanceFromHead)
    {
        FVector ExtendDirection;
        FVector ExtendFrom;
        
        if (ValidPoints.Num() >= 2)
        {
            // 마지막 세그먼트의 방향으로 연장
            ExtendDirection = (ValidPoints[0].Location - ValidPoints[1].Location).GetSafeNormal();
            ExtendFrom = ValidPoints[0].Location;
            
            // 방향이 유효하지 않으면 캐릭터의 뒤쪽 방향 사용
            if (ExtendDirection.IsNearlyZero())
            {
                ExtendDirection = -OwnerCharacter->GetActorForwardVector();
            }
        }
        else
        {
            // 궤적이 거의 없으면 캐릭터 뒤쪽으로 배치
            ExtendDirection = -OwnerCharacter->GetActorForwardVector();
            ExtendFrom = OwnerCharacter->GetActorLocation();
        }
        
        float RemainingDistance = DistanceFromHead - AccumulatedDistance;
        OutPosition = ExtendFrom + ExtendDirection * RemainingDistance;
        OutRotation = ExtendDirection.Rotation();
    }
}

void UAquaticMovementComponent::UpdateBoneTrailsDistanceBased(float DeltaTime)
{
    if (!CharacterMesh) return;

    // 본 간 거리 계산이 안되어 있으면 계산
    if (!bBoneDistanceCalculated)
    {
        CalculateAverageBoneDistance();
    }
    
    // 첫 번째 본 위치 (머리 또는 현재 위치)
    FVector HeadPosition = GetHeadBoneLocation();
    
    for (int32 i = 0; i < TrackedBones.Num(); i++)
    {
        FBoneTrailInfo& BoneInfo = TrackedBones[i];
        if (BoneInfo.BoneName.IsNone()) continue;

        // 본의 컴포넌트 공간 위치
        int32 BoneIndex = CharacterMesh->GetBoneIndex(BoneInfo.BoneName);
        if (BoneIndex == INDEX_NONE) continue;

        FTransform BoneWorldTransform = CharacterMesh->GetBoneTransform(BoneIndex);
        
        // 원래 본 위치 계산 (오프셋 제거)
        FVector BoneOriginalLocation = GetOriginalBoneLocation(i);
        
        // 머리로부터의 거리 계산 - 항상 일정한 간격 유지
        float DistanceFromHead = AverageBoneDistance * (i + 1);
        
        // 경로상의 목표 위치와 회전
        FVector TrailPosition;
        FRotator TrailRotation;
        GetPositionAndRotationAlongPath(DistanceFromHead, TrailPosition, TrailRotation);
        
        // 디버그 로그
        if (i == 0 && bDrawBoneTrails)
        {
            UE_LOG(LogAquaticMovement, Warning, TEXT("Distance-based: Bone[%d] Distance=%.1f, TrailPos=%s"), 
                i, DistanceFromHead, *TrailPosition.ToString());
        }
        
        // 위치 델타 계산 (원래 본 위치 기준)
        FVector ToTrailPosition = TrailPosition - BoneOriginalLocation;
        float DistanceToTrail = ToTrailPosition.Size();
        
        // 거리 기반 오프셋 계산
        float OffsetStrength = FMath::Clamp(DistanceToTrail / (AverageBoneDistance * 2.0f), 0.0f, 1.0f);
        FVector WorldOffset = ToTrailPosition * BoneInfo.PositionStrength * OffsetStrength;
        
        // 최대 오프셋 제한
        float MaxOffset = AverageBoneDistance * 1.0f;
        if (WorldOffset.Size() > MaxOffset)
        {
            WorldOffset = WorldOffset.GetSafeNormal() * MaxOffset;
        }
        
        // 메시 공간으로 변환
        FVector MeshSpaceDelta = CharacterMesh->GetComponentTransform().InverseTransformVector(WorldOffset);
        
        // 부드러운 전환
        BoneInfo.PositionDelta = FMath::VInterpTo(
            BoneInfo.PositionDelta,
            MeshSpaceDelta,
            DeltaTime,
            BoneOffsetSmoothSpeed
        );
        
        // 회전 델타 (Roll 보존)
        FQuat CurrentWorldRot = BoneWorldTransform.GetRotation();
        FRotator CurrentRotator = CurrentWorldRot.Rotator();
        
        // Trail 회전에서 Roll은 현재 본의 Roll로 유지
        FRotator ModifiedTrailRotation = TrailRotation;
        ModifiedTrailRotation.Roll = CurrentRotator.Roll;
        
        FQuat TargetWorldRot = ModifiedTrailRotation.Quaternion();
        FQuat DeltaRot = TargetWorldRot * CurrentWorldRot.Inverse();
        
        FRotator TargetRotDelta = DeltaRot.Rotator() * BoneInfo.RotationStrength;
        BoneInfo.RotationDelta = FMath::RInterpTo(
            BoneInfo.RotationDelta,
            TargetRotDelta,
            DeltaTime,
            BoneOffsetSmoothSpeed * 0.5f
        );
    }
}

// === 메시 기반 경로 함수들 ===
FVector UAquaticMovementComponent::GetOriginalBoneLocation(int32 BoneArrayIndex) const
{
    if (!CharacterMesh || BoneArrayIndex < 0 || BoneArrayIndex >= TrackedBones.Num())
        return OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
    
    const FBoneTrailInfo& BoneInfo = TrackedBones[BoneArrayIndex];
    int32 BoneIndex = CharacterMesh->GetBoneIndex(BoneInfo.BoneName);
    if (BoneIndex != INDEX_NONE)
    {
        // 현재 본 위치에서 적용된 오프셋을 제거
        FVector CurrentLocation = CharacterMesh->GetBoneLocation(BoneInfo.BoneName);
        FVector WorldOffset = CharacterMesh->GetComponentTransform().TransformVector(BoneInfo.PositionDelta);
        return CurrentLocation - WorldOffset;
    }
    
    return OwnerCharacter->GetActorLocation();
}

FVector UAquaticMovementComponent::GetHeadBoneLocation() const
{
    // 오프셋이 제거된 첫 번째 본의 원래 위치
    return GetOriginalBoneLocation(0);
}

FVector UAquaticMovementComponent::GetTailBoneLocation() const
{
    // 오프셋이 제거된 마지막 본의 원래 위치
    return GetOriginalBoneLocation(TrackedBones.Num() - 1);
}


// === 개별 본 조회 함수들 ===
FVector UAquaticMovementComponent::GetBoneWorldOffset(const FName& BoneName) const
{
    for (const FBoneTrailInfo& BoneInfo : TrackedBones)
    {
        if (BoneInfo.BoneName == BoneName)
        {
            // 로컬 오프셋을 월드 공간으로 변환
            //return OwnerCharacter->GetActorTransform().TransformVector(BoneInfo.CurrentOffset);
        }
    }

    return FVector::ZeroVector;
}

FRotator UAquaticMovementComponent::GetBoneRotationOffset(const FName& BoneName) const
{
    for (const FBoneTrailInfo& BoneInfo : TrackedBones)
    {
        if (BoneInfo.BoneName == BoneName)
        {
           // return BoneInfo.CurrentRotationOffset;
        }
    }

    return FRotator::ZeroRotator;
}

FTransform UAquaticMovementComponent::GetBoneTrailTransform(const FName& BoneName) const
{
    for (const FBoneTrailInfo& BoneInfo : TrackedBones)
    {
        if (BoneInfo.BoneName == BoneName)
        {
           // FVector Offset = BoneInfo.CurrentOffset;
            //FRotator Rotation = BoneInfo.CurrentRotationOffset;

            // 로컬 트랜스폼 반환
            //return FTransform(Rotation, Offset);
        }
    }

    return FTransform::Identity;
}


// === 디버그 시각화 ===
void UAquaticMovementComponent::DrawDebugVisualization()
{
    if (!GetWorld()) return;

    // 경로 그리기
    if (bDrawTrajectory && TrajectoryHistory.Num() > 1)
    {
        // 경로 선 그리기
        for (int32 i = 1; i < TrajectoryHistory.Num(); i++)
        {
            FColor LineColor = FColor::Cyan;

            // 중요도에 따라 색상 변경
            if (TrajectoryHistory[i].Importance < 0.5f)
            {
                LineColor = FColor::Blue; // 낮은 중요도
            }
            else if (TrajectoryHistory[i].Importance > 0.8f)
            {
                LineColor = FColor::Yellow; // 높은 중요도
            }

            DrawDebugLine(GetWorld(),
                TrajectoryHistory[i - 1].Location,
                TrajectoryHistory[i].Location,
                LineColor, false, -1.0f, 0, 2.0f);
                

            // 방향 표시
            if (i % 5 == 0) // 매 5번째 포인트마다
            {
                FVector Direction = TrajectoryHistory[i].Rotation.Vector();
                DrawDebugDirectionalArrow(GetWorld(),
                    TrajectoryHistory[i].Location,
                    TrajectoryHistory[i].Location + Direction * 50.0f,
                    20.0f, LineColor, false, -1.0f, 0, 2.0f);
            }
        }

        // 현재 위치 표시
        DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), 20.0f, 12, FColor::Green, false, -1.0f);
        
        // 현재 머리/꼬리 표시
        FVector CurrentHead = GetHeadBoneLocation();
        FVector CurrentTail = GetTailBoneLocation();
        DrawDebugSphere(GetWorld(), CurrentHead, 15.0f, 8, FColor::Red, false, -1.0f);
        DrawDebugSphere(GetWorld(), CurrentTail, 15.0f, 8, FColor::Blue, false, -1.0f);
        DrawDebugLine(GetWorld(), CurrentHead, CurrentTail, FColor::Magenta, false, -1.0f, 0, 4.0f);
    }

    // 본 트레일 그리기
    if (bDrawBoneTrails)
    {
        for (int32 i=0; i<TrackedBones.Num(); i++)//const FBoneTrailInfo& BoneInfo : TrackedBones)
        {
            
            const FBoneTrailInfo& BoneInfo = TrackedBones[i];
            // 본의 현재 위치
            FVector BoneLocation = CharacterMesh->GetBoneLocation(BoneInfo.BoneName);

            // 트레일 위치 (거리 기반)
            float DistanceFromHead = AverageBoneDistance * (i + 1);
            FVector TrailPosition;
            FRotator TrailRotation;
            GetPositionAndRotationAlongPath(DistanceFromHead, TrailPosition, TrailRotation);

            // 연결선 그리기
            DrawDebugLine(GetWorld(), BoneLocation, TrailPosition, FColor::Yellow, false, -1.0f, 0, 1.0f);

            // 트레일 위치 구체
            DrawDebugSphere(GetWorld(), TrailPosition, 8.0f, 8, FColor::Orange, false, -1.0f);
        }
    }

    // 장애물 회피 시각화
    if (bDrawAvoidance)
    {
        FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        FVector Forward = OwnerCharacter->GetActorForwardVector();

        // 레이캐스트 표시
        for (int32 i = 0; i < AvoidanceRayCount; i++)
        {
            float Angle = (360.0f / AvoidanceRayCount) * i;
            FVector RayDirection = Forward.RotateAngleAxis(Angle, FVector::UpVector);
            FVector RayEnd = CurrentLocation + RayDirection * ObstacleDetectionRange;

            FHitResult Hit;
            bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CurrentLocation, RayEnd, ECC_WorldStatic);

            DrawDebugLine(GetWorld(), CurrentLocation, bHit ? Hit.Location : RayEnd,
                bHit ? FColor::Red : FColor::Green, false, -1.0f, 0, 1.0f);

            if (bHit)
            {
                DrawDebugPoint(GetWorld(), Hit.Location, 10.0f, FColor::Red, false, -1.0f);
            }
        }

        // 회피 위치 표시
        for (const FObstacleInfo& Obstacle : ActiveObstacles)
        {
            DrawDebugSphere(GetWorld(), Obstacle.Location, Obstacle.Radius, 16, FColor::Purple, false, -1.0f);

            // 영향 범위
            DrawDebugSphere(GetWorld(), Obstacle.Location, Obstacle.Radius * 2.0f, 16, FColor::Purple.WithAlpha(64), false, -1.0f);
        }

        // 회피 힘 벡터
        FVector AvoidanceForce = CalculateAvoidanceForce();
        if (!AvoidanceForce.IsNearlyZero())
        {
            DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
                CurrentLocation + AvoidanceForce.GetSafeNormal() * 200.0f,
                50.0f, FColor::Magenta, false, -1.0f, 0, 3.0f);
        }
        
        // 목표 방향 표시
        if (bHasTarget)
        {
            FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
            DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
                CurrentLocation + ToTarget * 300.0f,
                30.0f, FColor::Yellow, false, -1.0f, 0, 2.0f);
        }
        
        // 벽면 타기 상태 표시
        if (CurrentWallFollowDirection != 0 && !LastWallNormal.IsZero())
        {
            FString WallFollowText = FString::Printf(TEXT("Wall Follow: %s"), 
                CurrentWallFollowDirection > 0 ? TEXT("Left-Hand") : TEXT("Right-Hand"));
            DrawDebugString(GetWorld(), CurrentLocation + FVector(0, 0, 150),
                WallFollowText, nullptr, FColor::Cyan, 0.0f, true);
            
            // 벽 법선 표시
            DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
                CurrentLocation + LastWallNormal * 100.0f,
                20.0f, FColor::Blue, false, -1.0f, 0, 2.0f);
        }
    }

    // 상태 정보 텍스트
    if (bDrawTrajectory || bDrawBoneTrails)
    {
        FString DebugText = FString::Printf(TEXT("Speed: %.1f / %.1f\nLOD: %.2f\nPoints: %d"),
            CurrentVelocity.Size(), MaxSpeed, CurrentLODFactor, TrajectoryHistory.Num());

        if (bHasTarget)
        {
            float DistanceToTarget = (TargetLocation - OwnerCharacter->GetActorLocation()).Size();
            DebugText += FString::Printf(TEXT("\nTarget Dist: %.1f"), DistanceToTarget);
        }

        DrawDebugString(GetWorld(), OwnerCharacter->GetActorLocation() + FVector(0, 0, 100),
            DebugText, nullptr, FColor::White, 0.0f, true);
    }
}