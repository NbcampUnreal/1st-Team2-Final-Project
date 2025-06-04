#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "EnhancedBossAIController.h"
#include "Enum/EBossPhysicsType.h"
#include "Enum/EBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Effect/CameraControllerComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Net/UnrealNetwork.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Enum/EPerceptionType.h"
#include "Perception/AISense_Damage.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"

const FName ABoss::BossStateKey = "BossState";

#pragma region 초기화 함수
ABoss::ABoss()
{
	bIsAttackCollisionOverlappedPlayer = false;
	BlackboardComponent = nullptr;
	AIController = nullptr;
	TargetPlayer = nullptr;
	LastDetectedLocation = FVector::ZeroVector;
	AttackRadius = 500.0f;
	LaunchPower = 1000.0f;
	MinPatrolDistance = 500.0f;
	MaxPatrolDistance = 1000.0f;
	AttackedCameraShakeScale = 1.0f;
	bIsBiteAttackSuccess = false;
	BossPhysicsType = EBossPhysicsType::None;
	DamagedLocation = FVector::ZeroVector;
	CachedSpawnLocation = FVector::ZeroVector;
	RotationInterpSpeed = 1.1f;
	bIsAttackInfinite = true;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	AttackCollision->SetCapsuleHalfHeight(80.0f);
	AttackCollision->SetCapsuleRadius(80.0f);
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->ComponentTags.Add(TEXT("Attack Collision"));

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>("Camera Controller Component");

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodNiagara(TEXT("/Game/SurvivalFX/Particles/Hit/PS_Hit_Blood_Big"));
	if (BloodNiagara.Succeeded())
	{
		BloodEffect = BloodNiagara.Object;
	}
	bReplicates = true;

	RadarReturnComponent->FactionTags.Init(TEXT("Hostile"), 1);
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();
	
	AnimInstance = GetMesh()->GetAnimInstance();

	AIController = Cast<ABossAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
	}

	CachedSpawnLocation = GetActorLocation();

	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapBegin);
	AttackCollision->OnComponentEndOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapEnd);

	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
	OriginDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;

	Params.AddIgnoredActor(this);
}

void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABoss::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABoss, BossState);
}
#pragma endregion

#pragma region 수중생물 AI 자체구현 함수
FVector ABoss::GetRandomNavMeshLocation(const FVector& Origin, const float& Radius) const
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

void ABoss::SetNewTargetLocation()
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

void ABoss::SmoothMoveAlongSurface(const float& InDeltaTime)
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
        const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRotation, InDeltaTime, RotationInterpSpeed);
        SetActorRotation(NewRot);
    }
    else
    {
        // 상하좌우 라인트레이싱이 모두 Hit이 아닌 경우에는 Roll을 0으로 보간하여 회전시킨다.
        // Roll이 회전된 상태로 이동하면 부자연스럽기 때문에 자연스러움을 연출시키기 위함이다.
        const FRotator CurrentRot = GetActorRotation();
        FRotator TargetRot = CurrentRot;
        TargetRot.Roll = 0.0f;

        const FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRot, InDeltaTime, RotationInterpSpeed);
        SetActorRotation(NewRotation);
    }
}

void ABoss::PerformNormalMovement(const float& InDeltaTime)
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
    const FVector NextLocation = CurrentLocation + CurrentForward * StatComponent->MoveSpeed * InDeltaTime;
    if (!IsLocationOnNavMesh(NextLocation))
    {
        StartTurn();
        return;
    }
    
    // 목표 회전 값이 90도보다 큰 경우 회전 속도를 절반으로 줄여 자연스러운 회전을 연출한다.
    float AdjustedRotationSpeed = RotationInterpSpeed;
    if (AngleDifference > 90.0f)
    {
        AdjustedRotationSpeed *= 0.5f;
    }
    
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, InDeltaTime, AdjustedRotationSpeed);
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
    const FVector NewLocation = CurrentLocation + GetActorForwardVector() * AdjustedSpeed * InDeltaTime;
    SetActorLocation(NewLocation, true);
}

void ABoss::StartTurn()
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

void ABoss::ReturnToNavMeshArea()
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

void ABoss::PerformTurn(const float& InDeltaTime)
{
	TurnTimer += InDeltaTime;
    
	// 목표 방향으로 회전
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = TurnDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, InDeltaTime, RotationInterpSpeed);
    
	SetActorRotation(NewRotation);
    
	// 회전 중일 때에도 느린 이동속도로 이동한다. 자연스러운 연출을 위함이다.
	const FVector NewLocation = GetActorLocation() + GetActorForwardVector() * StatComponent->MoveSpeed * 0.4f * InDeltaTime;
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

bool ABoss::HasObstacleAhead()
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

bool ABoss::IsLocationOnNavMesh(const FVector& InLocation) const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavSystem)) return false;
    
	FNavLocation NavLocation;

	// 액터가 NavMesh 상에 존재하는지에 대한 여부를 반환한다.
	return NavSystem->ProjectPointToNavigation(InLocation, NavLocation, FVector(100.0f, 100.0f, 100.0f));
}
#pragma endregion

#pragma region 보스 상태 관련 함수
void ABoss::SetCharacterMovementSetting(const float& InBrakingDecelerationSwimming, const float& InMaxSwimSpeed)
{
	if (!IsValid(GetCharacterMovement())) return;

	if (InBrakingDecelerationSwimming < 0.0f || InMaxSwimSpeed <= 0.0f) return;

	GetCharacterMovement()->BrakingDecelerationSwimming = InBrakingDecelerationSwimming;
	GetCharacterMovement()->MaxSwimSpeed = InMaxSwimSpeed;
}

void ABoss::InitCharacterMovementSetting()
{
	if (!IsValid(GetCharacterMovement())) return;
	
	GetCharacterMovement()->BrakingDecelerationSwimming = OriginDeceleration;
	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
}

void ABoss::SetBossState(EBossState State)
{
	if (!HasAuthority()) return;

	BossState = State;
	BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));
}
#pragma endregion

#pragma region  TakeDamage, Death
float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
                        AActor* DamageCauser)
{
	// 사망 상태면 얼리 리턴
	if (BossState == EBossState::Death) return 0.0f;
	
	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 부위 타격 정보
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamage = static_cast<const FPointDamageEvent*>(&DamageEvent);
		
		FHitResult HitResult = PointDamage->HitInfo;
		if (HitResult.BoneName != NAME_None)
		{
			LOG(TEXT("Hit Bone: %s"), *HitResult.BoneName.ToString());
		}

		if (HitResult.PhysicsObjectOwner.IsValid())
		{
			FName RegionName = *HitResult.PhysicsObjectOwner->GetName();
			LOG(TEXT("%s"), *RegionName.ToString());			
		}

		if (HitResult.ImpactPoint != FVector::ZeroVector)
		{
			LOG(TEXT("Damage Location: %s"), *HitResult.ImpactPoint.ToString());
			if (IsValid(BloodEffect))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), BloodEffect,HitResult.ImpactPoint, FRotator::ZeroRotator, FVector(1), true, true );
			}
		}

		UAISense_Damage::ReportDamageEvent(
		GetWorld(),
		this,        // 데미지를 입은 보스
		DamageCauser,      // 공격한 플레이어
		Damage,
		DamageCauser->GetActorLocation(),
		HitResult.ImpactPoint
		);

		DamagedLocation = DamageCauser->GetActorLocation();
	}
	
	if (IsValid(StatComponent))
	{
		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
		}
	}
	return Damage;
}

void ABoss::OnDeath()
{
	GetCharacterMovement()->StopMovementImmediately();

	// 피직스 에셋 물리엔진 적용
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABoss::ApplyPhysicsSimulation, 0.5f, false);	

	// 이동을 멈추고 모든 애니메이션 출력 정지
	AIController->StopMovement();
	AnimInstance->StopAllMontages(0.5f);
	
	// 사망 상태로 전이
	SetBossState(EBossState::Death);

	// AIController 작동 중지
	AIController->UnPossess();
}
#pragma endregion

#pragma region 보스 유틸 함수
void ABoss::LaunchPlayer(AUnderwaterCharacter* Player, const float& Power) const
{
	// 플레이어를 밀치는 로직
	const FVector PushDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const float PushStrength = Power;
	const FVector PushForce = PushDirection * PushStrength;
	
	// 물리 시뮬레이션이 아닌 경우 LaunchCharacter 사용
	Player->LaunchCharacter(PushForce, false, false);

	// 0.5초 후 캐릭터의 원래 움직임 복구
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Player]()
	{
		if (IsValid(Player))
		{
			Player->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);	
		}
	}, 0.5f, false);
}

void ABoss::RotationToTarget(AActor* Target)
{
	if (!IsValid(Target)) return;

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (Target->GetActorLocation() - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	
	SetActorRotation(NewRotation);
}

void ABoss::RotationToTarget(const FVector& InTargetLocation)
{
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (InTargetLocation - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	
	SetActorRotation(NewRotation);
}

void ABoss::Attack()
{
	const uint8 AttackType = FMath::RandRange(0, NormalAttackAnimations.Num() - 1);
	
	if (IsValid(NormalAttackAnimations[AttackType]))
	{
		ChaseAccumulatedTime = 0.f;
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABoss::OnAttackMontageEnded);
		M_PlayAnimation(NormalAttackAnimations[AttackType]);
	}
}

void ABoss::OnAttackEnded()
{
	AttackedPlayers.Empty();
}

void ABoss::SetMoveSpeed(const float& SpeedMultiplier)
{
	GetCharacterMovement()->MaxFlySpeed = StatComponent->MoveSpeed * SpeedMultiplier;
}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void ABoss::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	AEnhancedBossAIController* EnhancedBossAIController = Cast<AEnhancedBossAIController>(GetController());
	if (!IsValid(EnhancedBossAIController) || !IsValid(AnimInstance)) return;

	if (bIsAttackInfinite)
	{
		EnhancedBossAIController->GetBlackboardComponent()->SetValueAsBool("bHasAttacked", false);
	}
	else
	{
		EnhancedBossAIController->GetBlackboardComponent()->SetValueAsBool("bHasDetectedPlayer", false);
		EnhancedBossAIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}
	
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABoss::OnAttackMontageEnded);
}

void ABoss::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 사망 상태면 얼리 리턴
	if (BossState == EBossState::Death) return;
	
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	// 해당 플레이어가 이미 공격받은 상태인 경우 얼리 리턴
	if (AttackedPlayers.Contains(Player)) return;

	// 공격받은 대상 리스트에 플레이어 추가
	AttackedPlayers.Add(Player);

	// 해당 플레이어에게 데미지 적용
	UGameplayStatics::ApplyDamage(Player, StatComponent->AttackPower, GetController(), this, UDamageType::StaticClass());

	// 피격당한 플레이어의 카메라 Shake
	CameraControllerComponent->ShakePlayerCamera(Player, AttackedCameraShakeScale);

	// 캐릭터 넉백
	LaunchPlayer(Player, LaunchPower);
}

void ABoss::OnBiteCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이미 Bite 한 대상이 있는 경우 얼리 리턴
	if (bIsBiteAttackSuccess) return;
	
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	// Bite 상태 변수 활성화 
	bIsBiteAttackSuccess = true;

	// 타겟 설정
	SetTarget(Player);
	Player->StartCaptureState();
}

void ABoss::OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;
	
	bIsAttackCollisionOverlappedPlayer = true;
}

void ABoss::OnAttackCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;
	
	bIsAttackCollisionOverlappedPlayer = false;
}

void ABoss::ApplyPhysicsSimulation()
{
	switch (BossPhysicsType)
	{
		// 물리엔진 비활성화
		case EBossPhysicsType::None:
			break;

		// Simulate Physic 적용
		case EBossPhysicsType::SimulatePhysics:
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetMesh()->SetEnableGravity(true);
			GetMesh()->SetSimulatePhysics(true);	
			break;
	}
}

FVector ABoss::GetNextPatrolPoint()
{
	// NavMesh 기반 랜덤 위치 찾기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavSys)) return GetActorLocation();

	FNavLocation RandomNavLocation;
	
	// 최대로 시도할 다음 경로 찾기 알고리즘 횟수
	const uint8 MaxTries = 20;

	const FVector CurrentLocation = GetActorLocation();

	for (uint8 i = 0; i < MaxTries; ++i)
	{
		const bool bFound = NavSys->GetRandomReachablePointInRadius(CurrentLocation, MaxPatrolDistance, RandomNavLocation);

		if (bFound)
		{
			const FVector NewTargetLocation = RandomNavLocation.Location;

			if (FVector::Distance(CurrentLocation, NewTargetLocation) > MinPatrolDistance)
			{
				return NewTargetLocation;
			}
		}	
	}

	return GetActorLocation();
}
#pragma endregion