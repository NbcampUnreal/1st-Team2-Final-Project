// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"	

#include "AquaticMovementComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAquaticMovement, Log, All);

// 경로상의 한 지점 정보
USTRUCT(BlueprintType)
struct FTrajectoryPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(BlueprintReadWrite)
    float Time;

    // LOD를 위한 중요도 (0-1)
    UPROPERTY(BlueprintReadWrite)
    float Importance;

    FTrajectoryPoint()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Time = 0.0f;
        Importance = 1.0f;
    }
};

// 본별 추적 정보
USTRUCT(BlueprintType)
struct FBoneTrailInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PositionStrength = 0.3f; // 위치 따라가기 강도 (낮게 시작)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RotationStrength = 0.8f; // 회전 따라가기 강도

    // 런타임 - 델타만 저장
    FVector PositionDelta;
    FRotator RotationDelta;
};

// 장애물 정보
USTRUCT()
struct FObstacleInfo
{
    GENERATED_BODY()

    FVector Location;
    float Radius;
    float AvoidanceStrength;
    float DetectionTime;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UAquaticMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAquaticMovementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === 초기화 ===
    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void InitComponent(class ACharacter* InCharacter);

    // === 이동 제어 ===

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void SetDesiredVelocity(const FVector& DesiredVel);

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void SetTargetLocation(const FVector& Target, float AcceptanceRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void StopMovement();

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void StopMovementImmediately();
    
    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    bool HasReachedTarget() const { return !bHasTarget; }
    
    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    float GetDistanceToTarget();

    // === 장애물 회피 ===

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void AddAvoidanceLocation(const FVector& Location, float Radius, float Strength = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement")
    void ClearAvoidanceLocations();

    // === 애니메이션 인터페이스 ===

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement|Animation")
    FVector GetBoneWorldOffset(const FName& BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement|Animation")
    FRotator GetBoneRotationOffset(const FName& BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement|Animation")
    FTransform GetBoneTrailTransform(const FName& BoneName) const;

    // 모든 본의 오프셋을 한번에 가져오기 (커스텀 노드용)
    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement|Animation")
    void GetAllBoneOffsets(TArray<FVector>& OutOffsets, TArray<FRotator>& OutRotations) const;

    UFUNCTION(BlueprintCallable, Category = "Aquatic Movement|Animation")
    void GetBoneDeltas(TArray<FVector>& OutPositionDeltas, TArray<FRotator>& OutRotationDeltas) const;

protected:
    // === 경로 관리 ===

    void UpdateTrajectory(float DeltaTime);
    void RecordTrajectoryPoint(float Importance = 1.0f);
    void OptimizeTrajectoryForLOD();
    void CleanOldTrajectoryPoints();
    void PredictFuturePath(float PredictionTime);

    // === 보간 ===

    void InterpolateTrajectory(float TimeDelay, FVector& OutPosition, FRotator& OutRotation) const;

    // === 본 추적 ===

    void CalculateAverageBoneDistance();
    
    // 거리 기반 시스템
    void UpdateBoneTrailsDistanceBased(float DeltaTime);
    void GetPositionAndRotationAlongPath(float DistanceFromHead, FVector& OutPosition, FRotator& OutRotation);
    
    // === 본 위치 ===

    FVector GetHeadBoneLocation();
    FVector GetTailBoneLocation();
    FVector GetOriginalBoneLocation(int32 BoneArrayIndex); // 오프셋 제거된 원래 본 위치

    // === 이동 업데이트 ===

    void UpdateMovement(float DeltaTime);
    FVector CalculateSteeringForce() const;
    FVector CalculateAvoidanceForce();

    // === LOD ===

    float GetLODFactor();
    float GetTrajectoryRecordInterval();

public:
    // === 이동 파라미터 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Acceleration = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnSpeed = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "10.0", ClampMax = "180.0"))
    float MaxTurnAnglePerSecond = 120.0f; // 초당 최대 회전 각도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BrakingDeceleration = 400.0f;

    // === 경로 설정 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxTrajectoryPoints = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float TrajectoryMaxHistory = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float BaseRecordInterval = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float TrajectoryPredictionTime = 1.5f; // 미래 예측 시간

    // 근거리/원거리 보간 전환 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float SplineInterpolationDistance = 1.0f; // 1초 이내는 스플라인

    // === 본 추적 설정 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Trail")
    TArray<FBoneTrailInfo> TrackedBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Trail")
    float BoneOffsetSmoothSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Trail")
    uint8 bShouldUpdateBoneTrails : 1 = true;;

    // === 장애물 회피 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    float ObstacleDetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    float ObstacleAvoidanceStrength = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    int32 AvoidanceRayCount = 8; // 회피용 레이케스트 쏘는 횟수
    
    // 벽면 타기 모드 (0: 자동 선택, 1: 좌수법, -1: 우수법)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    int32 WallFollowingMode = 0;
    
    // 목표 방향의 장애물에 대한 가중치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TargetDirectionWeight = 1.5f;

    // === LOD 설정 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceMin = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceMax = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODMinRecordInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
    float CurrentLODFactor;

    // === 디버그 ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    uint8 bDrawTrajectory : 1 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    uint8 bDrawBoneTrails : 1 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    uint8 bDrawAvoidance : 1 = false;

    // === 상태 ===   

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector DesiredVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector TargetLocation;

protected:
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    uint8 bHasTarget : 1 = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    float TargetAcceptanceRadius;

    // === 경로 데이터 ===

    UPROPERTY()
    TArray<FTrajectoryPoint> TrajectoryHistory;

    // === 장애물 데이터 ===

    UPROPERTY()
    TArray<FObstacleInfo> ActiveObstacles;

    // === 캐시 ===

    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> CharacterMesh;

    UPROPERTY()
    TWeakObjectPtr<APlayerController> LocalPlayerController;

    UPROPERTY()
    TSet<TObjectPtr<AActor>> IgnoredActors;
    FCollisionQueryParams AvoidanceTraceParams;

    float LastRecordTime;
    FVector LastRecordedLocation;
    float AverageBoneDistance;
    uint8 bBoneDistanceCalculated : 1 = false;
    
    // 벽면 타기 상태 캐시
    int32 CurrentWallFollowDirection; // 현재 벽면 타기 방향 (-1, 0, 1)
    float LastWallDetectionTime; // 마지막 벽 감지 시간
    FVector LastWallNormal; // 마지막 벽 법선

private:

    // === 스플라인 보간 ===

    FVector CatmullRomSpline(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T) const;

    // === 디버그 ===
    // Avoidance 관련 디버그는 단순 Hit 체크만 하기 때문에 실제로 무시되는 대상도 빨간색으로 표시될 수 있다.
    void DrawDebugVisualization();

    void AddIgnoredActorFromAvoidance(AActor* Actor);

private:

    ACharacter* GetOwnerCharacter();
    USkeletalMeshComponent* GetOwnerMesh();
    APlayerController* GetLocalPlayerController();
};