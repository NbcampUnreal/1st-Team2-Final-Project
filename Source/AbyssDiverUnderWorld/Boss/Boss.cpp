#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "EnhancedBossAIController.h"
#include "NavigationPath.h"
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
#include "Framework/ADPlayerController.h"
#include "Perception/AISense_Damage.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"
#include "Missions/MissionEventHubComponent.h"
#include "GameFramework/GameStateBase.h"



const FName ABoss::BossStateKey = "BossState";

#pragma region 초기화 함수
ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bIsAttackCollisionOverlappedPlayer = false;
	TargetPlayer = nullptr;
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

	EnhancedAIController = Cast<AEnhancedBossAIController>(GetController());
	
	CachedSpawnLocation = GetActorLocation();

	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapBegin);
	AttackCollision->OnComponentEndOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapEnd);

	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
	OriginDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;
	CurrentMoveSpeed = StatComponent->MoveSpeed;

	SphereOverlapRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;

	Params.AddIgnoredActor(this);
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

    	// 경로상의 중간 지점들도 NavMesh 내에 있는지 확인
    	bool bPathClear = true;
    	const int32 CheckPoints = 5;
    	for (int32 i = 1; i <= CheckPoints; i++)
    	{
    		FVector CheckPoint = FMath::Lerp(CurrentLocation, PotentialTarget, (float)i / CheckPoints);
    		if (!IsLocationOnNavMesh(CheckPoint))
    		{
    			bPathClear = false;
    			break;
    		}
    	}
        
    	if (!bPathClear)
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

#if WITH_EDITOR
        	if (bDrawDebugLine)
        	{
        		DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Yellow, false, 3.0f, 0, 5.0f);
        		DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, FColor::Yellow, false, 3.0f, 0, 3.0f);
        	}
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
    	if (bDrawDebugLine)
    	{
    		DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Cyan, false, 3.0f, 0, 5.0f);
    		DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, FColor::Cyan, false, 3.0f, 0, 3.0f);	
    	}

#endif
        
        return;
    }

	// 최후의 수단으로 가까운 NavMesh 지점 설정
	TargetLocation = GetRandomNavMeshLocation(CurrentLocation, MinTargetDistance * 3.0f);
	if (FVector::Dist(TargetLocation, CurrentLocation) <= KINDA_SMALL_NUMBER)
	{
		// NavMesh에서 안전한 지점 찾기
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (IsValid(NavSystem))
		{
			FNavLocation SafeLocation;
			if (NavSystem->ProjectPointToNavigation(CurrentLocation, SafeLocation, FVector(500.0f, 500.0f, 500.0f)))
			{
				TargetLocation = SafeLocation.Location + Forward * MinTargetDistance;
			}
		}
	}
    LOG(TEXT("Fallback target set: %s"), *TargetLocation.ToString());

#if WITH_EDITOR
	if (bDrawDebugLine)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
	}
#endif
}

void ABoss::SmoothMoveAlongSurface(const float& InDeltaTime)
{
	bool bHit = false;
    FHitResult NearestHit;
    float NearestDistance = TNumericLimits<float>::Max();
    
    // 월드 기준 절대적인 방향으로 변경
    TArray<FVector> Directions;

	if (bEnableDownTrace)
	{
		Directions.Emplace(-FVector::UpVector * 1.75f);
	}

	if (bEnableHorizontalTrace)
	{
		Directions.Emplace(FVector::RightVector);
		Directions.Emplace(-FVector::RightVector);
		Directions.Emplace(FVector::ForwardVector);
		Directions.Emplace(-FVector::ForwardVector);
	}
	
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
        if (bDrawDebugLine)
        {
           DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.1f, 0, 2.0f);
        }
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
        SetNewTargetLocation();
        return;
    }
    
    const float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    
    // 목표점에 도달했으면 새 목표점 설정
    if (DistanceToTarget < MinTargetDistance)
    {
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
    if (DistanceToTarget > WanderRadius * 2.0f)
    {
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
	CurrentMoveSpeed = FMath::FInterpTo(CurrentMoveSpeed, AdjustedSpeed, InDeltaTime, MovementInterpSpeed);
    const FVector NewLocation = CurrentLocation + GetActorForwardVector() * CurrentMoveSpeed * InDeltaTime;
    SetActorLocation(NewLocation, true);
}

void ABoss::PerformChasing(const float& InDeltaTime)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(EnhancedAIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player)) return;

	const FVector CurrentLocation = GetActorLocation();
	const FVector PlayerLocation = Player->GetActorLocation();

	// NavMesh를 사용한 경로 탐색
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavSystem)) return;

	// 경로 찾기
	UNavigationPath* NavPath = NavSystem->FindPathToLocationSynchronously(
		GetWorld(),
		CurrentLocation,
		PlayerLocation,
		this,
		TSubclassOf<UNavigationQueryFilter>()
	);

	FVector TargetDirection;

	if (IsValid(NavPath) && NavPath->PathPoints.Num() > 1)
	{
		// 경로가 존재하는 경우, 다음 경로점으로 방향 설정
		// PathPoints[0]은 현재 위치, PathPoints[1]이 다음 목표점
		const FVector NextPathPoint = NavPath->PathPoints[1];
		TargetDirection = (NextPathPoint - CurrentLocation).GetSafeNormal();
    
		// 경로점에 충분히 가까워졌다면 그 다음 점을 타겟으로 설정
		const float DistanceToNextPoint = FVector::Dist(CurrentLocation, NextPathPoint);
		if (DistanceToNextPoint < 100.0f && NavPath->PathPoints.Num() > 2)
	    {
	        const FVector SecondNextPoint = NavPath->PathPoints[2];
	        TargetDirection = (SecondNextPoint - CurrentLocation).GetSafeNormal();
	    }
	}
	else
	{
	    // 경로가 없는 경우 직접 플레이어 방향으로 (기존 로직)
	    TargetDirection = (PlayerLocation - CurrentLocation).GetSafeNormal();
	}

	// 타겟 방향으로 회전
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = TargetDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, InDeltaTime, RotationInterpSpeed * ChasingRotationSpeedMultiplier);
	SetActorRotation(NewRotation);

	// 전방벡터로 이동 (기존 로직 유지)
	const float AdjustedMoveSpeed = StatComponent->MoveSpeed * ChasingMovementSpeedMultiplier;
	CurrentMoveSpeed = FMath::FInterpTo(CurrentMoveSpeed, AdjustedMoveSpeed, InDeltaTime, MovementInterpSpeed);
	const FVector NewLocation = CurrentLocation + GetActorForwardVector() * CurrentMoveSpeed * InDeltaTime;
	SetActorLocation(NewLocation, true);

	//Mission Event Hub에 어그로 이벤트 브로드캐스트
	if (UMissionEventHubComponent* Hub = GetMissionHub())
	{
		Hub->BroadcastAggroTriggered(GameplayTags);
	}
}

void ABoss::StartTurn()
{
	EnhancedAIController->GetBlackboardComponent()->SetValueAsBool("bIsTurning", true);
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

    	// 후방 수평 방향들
    	Forward.RotateAngleAxis(120.0f, Up),
	    Forward.RotateAngleAxis(-120.0f, Up),
	    Forward.RotateAngleAxis(150.0f, Up),
	    Forward.RotateAngleAxis(-150.0f, Up),

    	// 대각선 방향들
		Forward.RotateAngleAxis(45.0f, Up).RotateAngleAxis(30.0f, Right),
		Forward.RotateAngleAxis(-45.0f, Up).RotateAngleAxis(30.0f, Right),
		Forward.RotateAngleAxis(45.0f, Up).RotateAngleAxis(-30.0f, Right),
		Forward.RotateAngleAxis(-45.0f, Up).RotateAngleAxis(-30.0f, Right),

		// 후방 대각선 방향들
    	Forward.RotateAngleAxis(135.0f, Up).RotateAngleAxis(30.0f, Right),
		Forward.RotateAngleAxis(-135.0f, Up).RotateAngleAxis(30.0f, Right),
		Forward.RotateAngleAxis(135.0f, Up).RotateAngleAxis(-30.0f, Right),
		Forward.RotateAngleAxis(-135.0f, Up).RotateAngleAxis(-30.0f, Right),

    	// 후방 방향
	    Forward.RotateAngleAxis(180.0f, Up),
    };
    
    // 수평, 수직, 대각선 방향에 대해 라인 트레이싱을 한다.
    // NavMesh를 벗어나지 않았고, 충돌이 발생하지 않는 방향을 목표 회전 값으로 저장한다.
    for (const FVector& Direction : PossibleDirections)
    {
    	const FVector Start = GetActorLocation();
    	const FVector End = Start + Direction * TurnTraceDistance;
        
    	// 목표 지점에서 Sphere Overlap 검사
    	const bool bOverlap = GetWorld()->OverlapAnyTestByChannel(
			End,                                                    // 검사할 위치
			FQuat::Identity,                                                   // 회전 없음
			ECC_Visibility,                                                    // 충돌 채널
			FCollisionShape::MakeSphere(SphereOverlapRadius), // Sphere 크기
			Params                                                             // 충돌 파라미터
		);

    	// NavMesh 검사
    	if (!IsLocationOnNavMesh(End))
    	{
    		continue;
    	}

#if WITH_EDITOR
    	// 디버그 표시
    	if (bDrawDebugLine)
    	{
    		// 방향 라인
    		DrawDebugLine(GetWorld(), Start, End, bOverlap ? FColor::Red : FColor::Green, false, 5.0f, 0, 3.0f);
        
    		// 목표 지점에 Sphere 표시
    		DrawDebugSphere(GetWorld(), End, SphereOverlapRadius, 
						   16, bOverlap ? FColor::Red : FColor::Green, false, 5.0f, 0, 2.0f);
    	}
#endif

    	// Overlap이 없으면 (빈 공간이면) 해당 방향 선택
    	if (!bOverlap)
    	{
    		TurnDirection = Direction;
    		return;
    	}
    }
	
	// 모든 방향이 막혔으면 뒤로 돌기
	TurnDirection = -GetActorForwardVector();
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
	}
}

void ABoss::PerformTurn(const float& InDeltaTime)
{
	TurnTimer += InDeltaTime;
    
	// 목표 방향으로 회전
	const FVector CurrentLocation = GetActorLocation();
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = TurnDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, InDeltaTime, RotationInterpSpeed);
    
	SetActorRotation(NewRotation);

	// 회전 중 이동할 위치 검증
	const float TurnMoveSpeed = StatComponent->MoveSpeed * 0.4f;
	CurrentMoveSpeed = FMath::FInterpTo(CurrentMoveSpeed, TurnMoveSpeed, InDeltaTime, MovementInterpSpeed);
	const FVector NextLocation = CurrentLocation + GetActorForwardVector() * CurrentMoveSpeed * InDeltaTime;
	
	// 이동할 위치가 NavMesh 내에 있고 장애물이 없는지 확인
	if (IsLocationOnNavMesh(NextLocation))
	{
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CurrentLocation,
			NextLocation,
			ECC_Visibility,
			Params
		);
        
		if (!bHit)
		{
			SetActorLocation(NextLocation, true);
		}
	}
	
	// 목표 회전 값과 현재 회전 값의 차이가 15도 미만이거나
	// 회전을 시작한 지 2.0초를 초과했다면 회전을 종료하고 목표 지점을 재설정한다.
	const float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));
	if (AngleDifference < 1.0f || TurnTimer > 2.0f)
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsBool("bIsTurning", false);
		TurnTimer = 0.0f;

		const bool bIsChasingBlood = EnhancedAIController->GetBlackboardComponent()->GetValueAsBool("bIsChasingBlood");
		if (!bIsChasingBlood)
		{
			SetNewTargetLocation();	
		}
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

	if (bDrawDebugLine)
	{
		DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.1f, 0, 3.0f);
	}
#endif
    
	return bPhysicalHit || bNavMeshBlocked;;
}

bool ABoss::IsLocationOnNavMesh(const FVector& InLocation) const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavSystem)) return false;
    
	FNavLocation NavLocation;
    
	// 더 작은 검색 범위로 정확한 NavMesh 검증
	const bool bOnNavMesh = NavSystem->ProjectPointToNavigation(InLocation, NavLocation, FVector(50.0f, 50.0f, 50.0f));
    
	if (!bOnNavMesh) return false;
    
	// 투영된 지점과 원래 지점의 거리가 너무 크면 NavMesh를 벗어난 것으로 판단
	const float DistanceToProjected = FVector::Dist(InLocation, NavLocation.Location);
	const float MaxAllowedDistance = 100.0f; // 허용 가능한 최대 거리
    
	return DistanceToProjected <= MaxAllowedDistance;
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
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		BossState = State;
		AIController->GetBlackboardComponent()->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));	
	}
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
			M_PlayBloodEffect(HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}

		UAISense_Damage::ReportDamageEvent(
		GetWorld(),
		this,        // 데미지를 입은 보스
		Cast<AADPlayerController>(DamageCauser)->GetCharacter(),      // 공격한 플레이어
		Damage,
		DamageCauser->GetActorLocation(),
		HitResult.ImpactPoint
		);
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

void ABoss::M_PlayBloodEffect_Implementation(const FVector& Location, const FRotator& Rotation)
{
	if (IsValid(BloodEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), BloodEffect, Location, Rotation, FVector(1), true, true);
	}
}

void ABoss::OnDeath()
{
	M_OnDeath();
	
	DeathToRaderOff();

	if (IsValid(GetController()))
	{
		GetController()->StopMovement();
		GetController()->UnPossess();	
	}

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
	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);
	
	if (IsValid(AttackAnimations[AttackType]))
	{
		ChaseAccumulatedTime = 0.f;
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABoss::OnAttackMontageEnded);
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABoss::OnAttackMontageEnded);
		M_PlayAnimation(AttackAnimations[AttackType]);
	}

	bIsAttacking = true;
}

void ABoss::OnAttackEnded()
{
	bIsAttacking = false; 
	AttackedPlayers.Empty();
}

void ABoss::SetMoveSpeed(const float& SpeedMultiplier)
{
	GetCharacterMovement()->MaxSwimSpeed = StatComponent->MoveSpeed * SpeedMultiplier;
}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void ABoss::M_OnDeath_Implementation()
{
	GetCharacterMovement()->StopMovementImmediately();

	if (IsValid(AnimInstance))
	{
		// 모든 애니메이션 출력 정지
		AnimInstance->StopAllMontages(0.5f);
	}
	
	
	// 사망 상태로 전이
	SetBossState(EBossState::Death);
	
	// 피직스 에셋 물리엔진 적용
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABoss::ApplyPhysicsSimulation, 0.5f, false);

	//Mission Event Hub에 몬스터 사망 이벤트 브로드캐스트
	if (UMissionEventHubComponent* Hub = GetMissionHub())
	{
		Hub->BroadcastMonsterKilled(GameplayTags);
	}
}

void ABoss::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsValid(EnhancedAIController)) return;
	
	if (bIsAttackInfinite)
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsBool("bHasAttacked", false);
	}
	else
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsBool("bHasDetectedPlayer", false);
		EnhancedAIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}
}

void ABoss::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 사망 상태면 얼리 리턴
	if (BossState == EBossState::Death) return;

	// 공격 가능한 상태가 아니라면 리턴
	if (!bIsAttacking) return;
	
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

void ABoss::DeathToRaderOff()
{
	URadarReturnComponent* RaderComponent = Cast<URadarReturnComponent>(GetComponentByClass(URadarReturnComponent::StaticClass()));
	if (RaderComponent)
	{
		RaderComponent->SetIgnore(true);
	}
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

UMissionEventHubComponent* ABoss::GetMissionHub()
{
	if (CachedHub) return CachedHub;

	// 1) GameState에서 바로 찾기 (권장)
	if (AGameStateBase* GS = UGameplayStatics::GetGameState(this))
	{
		if (!CachedHub)
		{
			CachedHub = GS->FindComponentByClass<UMissionEventHubComponent>();
		}
	}

	return CachedHub;
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