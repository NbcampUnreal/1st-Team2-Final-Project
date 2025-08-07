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
#include "Outsourced/AquaticMovementComponent.h"

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

	// 새로운 물리 기반 수중 이동 컴포넌트 초기화
	AquaticMovementComponent = CreateDefaultSubobject<UAquaticMovementComponent>("Aquatic Movement Component");

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
	
	// 블랙보드의 TargetLocation 초기화 (Invalid 방지)
	if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", GetActorLocation());
	}

	// AquaticMovementComponent 초기화
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->InitializeComponent(this);
		
		// Boss의 기본 이동 속도 설정 (컴포넌트 변수를 무시하고 CharcterMovementComponent의 값을 사용하려면 주석 제거)
		//AquaticMovementComponent->MaxSpeed = GetCharacterMovement()->MaxSwimSpeed;
		//AquaticMovementComponent->Acceleration = 200.0f;
		//AquaticMovementComponent->TurnSpeed = RotationInterpSpeed * 30.0f; // 도/초 단위로 변환
		//AquaticMovementComponent->BrakingDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;
	}

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
	// [DEPRECATED] - NavMesh는 더 이상 사용되지 않습니다.
	// 라인 트레이스 기반 위치 탐색은 SetNewTargetLocation()에서 처리됩니다.
	/*
	   [DEPRECATED] 기존 NavMesh 랜덤 위치 시스템
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
    */
	return Origin;
}

void ABoss::SetNewTargetLocation()
{
	const FVector CurrentLocation = GetActorLocation();
	
	// 현재 전방 방향을 기준으로 목표 찾기
	FVector Forward = FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.0f).GetSafeNormal();	
	const FVector Right = GetActorRightVector();
	const FVector Up = GetActorUpVector();
	
	// 연속적인 이동을 위해 현재 속도 방향도 고려
	if (AquaticMovementComponent && !AquaticMovementComponent->CurrentVelocity.IsZero())
	{
		// 현재 이동 방향과 전방 방향을 블렌드하여 자연스러운 경로 생성
		const FVector VelocityDirection = AquaticMovementComponent->CurrentVelocity.GetSafeNormal();
		Forward = (Forward + VelocityDirection * 0.5f).GetSafeNormal();
	}
	
	// 현재 캐릭터의 피치 각도 가져오기
	const FRotator CurrentRotation = GetActorRotation();
	const float CurrentPitch = CurrentRotation.Pitch;
	
	// TargetPlayer 존재 여부 확인
	const bool bHasTargetPlayer = EnhancedAIController && 
		EnhancedAIController->GetBlackboardComponent() &&
		EnhancedAIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer") != nullptr;
	
	// 순찰 시 회전 제한, 그러나 배회를 위해 전방향도 가능
	const float MaxHorizontalAngle = 180.0f;//bHasTargetPlayer ? 90.0f : 60.0f;
	
	// 전방 시야각 내에서 도달 가능한 지점 찾기
	for (uint8 Attempts = 0; Attempts < 10; Attempts++)
	{
		// 연속성을 위해 전방 위주로 각도 제한
		float HorizontalAngle;
		if (!bHasTargetPlayer && Attempts < 10)
		{
			// 순찰 시 처음 10번은 더 전방 위주로
			HorizontalAngle = FMath::RandRange(-MaxHorizontalAngle * 0.5f, MaxHorizontalAngle * 0.5f);
		}
		else
		{
			HorizontalAngle = FMath::RandRange(-MaxHorizontalAngle, MaxHorizontalAngle);
		}
		
		// 현재 피치를 기준으로 ±20도만 허용
		const float VerticalAngleLimit = 20.0f;
		const float VerticalAngle = FMath::RandRange(-VerticalAngleLimit, VerticalAngleLimit);
		
		// 새로운 방향 벡터 생성
		// 회전은 월드 기준으로 (수중에서도 상하 개념 유지, 뒤집힘 방지)
		FVector NewDirection = Forward.RotateAngleAxis(HorizontalAngle, FVector::UpVector);
		const FVector RotatedRight = FVector::CrossProduct(FVector::UpVector, NewDirection).GetSafeNormal();
		NewDirection = NewDirection.RotateAngleAxis(VerticalAngle, RotatedRight);
		NewDirection.Normalize();
		
		// 순찰 시 항상 충분한 거리 확보
		float MinDistance = bHasTargetPlayer ? WanderRadius * 0.5f : WanderRadius * 0.8f;
		float MaxDistance = bHasTargetPlayer ? WanderRadius * 1.2f : WanderRadius * 1.5f;
		const float RandomDistance = FMath::RandRange(MinDistance, MaxDistance);
		FVector PotentialTarget = CurrentLocation + NewDirection * RandomDistance;
		
		// 라인 트레이스로 장애물 확인
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CurrentLocation + NewDirection * 100.0f, // 약간 전방에서 시작 (충돌 방지)
			PotentialTarget,
			ECC_Visibility,
			Params
		);
		// 장애물이 없으면 해당 위치를 목표로 설정

		if (!bHit)
		{
			DesiredTargetLocation = PotentialTarget;
			
			// 블랙보드의 TargetLocation 업데이트
			if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
			{
				EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
			}
			
#if WITH_EDITOR
			if (bDrawDebugLine)
			{
				DrawDebugSphere(GetWorld(), DesiredTargetLocation, 50.0f, 12, FColor::Cyan, false, 3.0f, 0, 5.0f);
				DrawDebugLine(GetWorld(), CurrentLocation, DesiredTargetLocation, FColor::Cyan, false, 3.0f, 0, 3.0f);
			}
#endif
			
			return;
		}
	}
	
	// 모든 시도가 실패하면 후방 및 측면으로 회전하여 새로운 방향 찾기
	for (uint8 BackupAttempts = 0; BackupAttempts < 5; BackupAttempts++)
	{
		// 후방 및 측면 방향 탐색 (360도 범위)
		const float HorizontalAngle = FMath::RandRange(-180.0f, 180.0f);
		// 수직 각도는 여전히 제한
		const float VerticalAngle = FMath::RandRange(-20.0f, 20.0f);
		
		// 수평 회전은 월드 Z축 기준으로
		FVector NewDirection = Forward.RotateAngleAxis(HorizontalAngle, FVector::UpVector);
		// 수직 회전은 회전된 방향의 오른쪽 벡터 기준으로
		const FVector RotatedRight = FVector::CrossProduct(FVector::UpVector, NewDirection).GetSafeNormal();
		NewDirection = NewDirection.RotateAngleAxis(VerticalAngle, RotatedRight);
		NewDirection.Normalize();
		
		const float RandomDistance = FMath::RandRange(WanderRadius * 0.3f, WanderRadius * 0.8f);
		FVector PotentialTarget = CurrentLocation + (NewDirection * 1000);
		
		// 라인 트레이스로 장애물 확인
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CurrentLocation + NewDirection * 100.0f,
			PotentialTarget,
			ECC_Visibility,
			Params
		);

		if (!bHit)
		{
			DesiredTargetLocation = PotentialTarget;  // TargetLocation이 아닌 DesiredTargetLocation 설정
			
			// 블랙보드의 TargetLocation 업데이트
			if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
			{
				EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
			}
			
#if WITH_EDITOR
			if (bDrawDebugLine)
			{
				DrawDebugSphere(GetWorld(), DesiredTargetLocation, 100.0f, 12, FColor::Blue, false, 3.0f, 0, 5.0f);
				DrawDebugLine(GetWorld(), CurrentLocation, DesiredTargetLocation, FColor::Blue, false, 3.0f, 0, 3.0f);
			}
#endif
			
			return;
		}
	}
	
	// 최후의 수단: 현재 위치에서 가장 가까운 빈 공간 찾기
	DesiredTargetLocation = CurrentLocation + Forward * MinTargetDistance;
	
#if WITH_EDITOR
	if (bDrawDebugLine)
	{
		DrawDebugSphere(GetWorld(), DesiredTargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
	}
#endif

	// 블랙보드의 TargetLocation 업데이트
	if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
	}

	DrawDebugSphere(GetWorld(), DesiredTargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
	DrawDebugLine(GetWorld(), CurrentLocation, DesiredTargetLocation, FColor::Red, false, 3.0f, 0, 3.0f);

}

void ABoss::SmoothMoveAlongSurface(const float& InDeltaTime)
{
/*
	   [DEPRECATED] 기존 표면 추적 시스템 - 새로운 AquaticSteeringMovementComponent로 대체 예정
	   bool bHit = false;
       FHitResult NearestHit;
       float NearestDistance = TNumericLimits<float>::Max();
    
        //월드 기준 절대적인 방향으로 변경
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
*/
}

void ABoss::PerformNormalMovement(const float& InDeltaTime)
{
	if (!AquaticMovementComponent)
	{
		LOG(TEXT("PerformNormalMovement: AquaticMovementComponent is null"));
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	
	// 보간된 타겟 위치가 초기화되지 않았으면 현재 위치로 설정
	if (InterpolatedTargetLocation.IsZero())
	{
		InterpolatedTargetLocation = CurrentLocation + GetActorForwardVector() * WanderRadius;
		DesiredTargetLocation = InterpolatedTargetLocation;
	}
	
	// 연속적인 이동을 위한 거리 체크
	const float DistanceToDesiredTarget = FVector::Dist(CurrentLocation, DesiredTargetLocation);
	const float DistanceToInterpolatedTarget = FVector::Dist(CurrentLocation, InterpolatedTargetLocation);
	
	// TargetPlayer가 없을 때만 연속 이동 적용
	const bool bHasTargetPlayer = EnhancedAIController && 
		EnhancedAIController->GetBlackboardComponent() &&
		EnhancedAIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer") != nullptr;
	
	if (!bHasTargetPlayer)
	{
		// 목표점에 가까워지면 미리 새 목표 설정 (멈추지 않고 계속 이동)
		if (DistanceToDesiredTarget < 500 || DesiredTargetLocation.IsZero())
		{
			SetNewTargetLocation();
		}
		
		// 목표점이 너무 멀면 재설정
		if (DistanceToDesiredTarget > WanderRadius * 2.0f)
		{
			SetNewTargetLocation();
		}
		
	}
	else
	{
		// TargetPlayer가 있으면 기존 로직 사용
		if (DesiredTargetLocation.IsZero() || AquaticMovementComponent->HasReachedTarget())
		{
			SetNewTargetLocation();
		}
	}
	
	// DesiredTargetLocation을 InterpolatedTargetLocation으로 부드럽게 보간
	if (!DesiredTargetLocation.IsZero())
	{
		// 순찰 시 더 부드러운 보간 속도 사용
		const float CurrentInterpSpeed = bHasTargetPlayer ? TargetLocationInterpSpeed : PatrolInterpSpeed;
		
		InterpolatedTargetLocation = FMath::VInterpTo(
			InterpolatedTargetLocation,
			DesiredTargetLocation,
			InDeltaTime,
			CurrentInterpSpeed
		);
		
		// 보간된 위치를 실제 타겟으로 설정
		TargetLocation = InterpolatedTargetLocation;
		AquaticMovementComponent->SetTargetLocation(TargetLocation, MinTargetDistance);
		
		// 블랙보드의 TargetLocation을 보간된 값으로 업데이트
		if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
		{
			EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", InterpolatedTargetLocation);
		}
		
#if WITH_EDITOR
		if (bDrawDebugLine)
		{
			// 보간된 타겟 위치를 보라색으로 표시
			DrawDebugSphere(GetWorld(), InterpolatedTargetLocation, 40.0f, 8, FColor::Purple, false, 0.1f);
			// 최종 목표 위치와의 선
			DrawDebugLine(GetWorld(), InterpolatedTargetLocation, DesiredTargetLocation, FColor::Purple, false, 0.1f, 0, 2.0f);
		}
#endif
	}
	
	// AquaticMovementComponent가 이동과 회전을 처리
	// TickComponent에서 자동으로 처리되므로 여기서는 추가 작업 불필요
}

void ABoss::PerformChasing(const float& InDeltaTime)
{
	if (!AquaticMovementComponent)
	{
		LOG(TEXT("PerformChasing: AquaticMovementComponent is null"));
		return;
	}
	
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(EnhancedAIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player))
	{
		LOG(TEXT("PerformChasing: Player is not valid"));
		return;
	}
	
	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector CurrentLocation = GetActorLocation();
	
	// 보간된 타겟 위치가 초기화되지 않았으면 현재 위치로 설정
	if (InterpolatedTargetLocation.IsZero())
	{
		InterpolatedTargetLocation = CurrentLocation;
	}
	
	// 플레이어까지 직선 경로가 막혀있는지 확인
	FHitResult HitResult;
	const bool bPathBlocked = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CurrentLocation,
		PlayerLocation,
		ECC_Visibility,
		Params
	);
	
	// 경로가 막혀있으면 우회 경로 탐색
	if (bPathBlocked && HitResult.GetActor() != Player)
	{
		// 장애물 주변으로 우회
		const FVector ObstacleLocation = HitResult.ImpactPoint;
		const FVector ToObstacle = (ObstacleLocation - CurrentLocation).GetSafeNormal();
		const FVector Right = FVector::CrossProduct(ToObstacle, FVector::UpVector);
		
		// 좌우 중 플레이어에 더 가까운 방향 선택
		const FVector LeftBypass = ObstacleLocation + Right * 300.0f;
		const FVector RightBypass = ObstacleLocation - Right * 300.0f;
		
		const float LeftDistance = FVector::Dist(LeftBypass, PlayerLocation);
		const float RightDistance = FVector::Dist(RightBypass, PlayerLocation);
		
		DesiredTargetLocation = (LeftDistance < RightDistance) ? LeftBypass : RightBypass;
	}
	else
	{
		// 직접 추적
		DesiredTargetLocation = PlayerLocation;
	}
	
	// DesiredTargetLocation을 InterpolatedTargetLocation으로 부드럽게 보간
	// 추적 시에는 더 빠른 보간 속도 사용
	const float ChasingInterpSpeed = TargetLocationInterpSpeed * 2.0f;
	InterpolatedTargetLocation = FMath::VInterpTo(
		InterpolatedTargetLocation,
		DesiredTargetLocation,
		InDeltaTime,
		ChasingInterpSpeed
	);
	
	// 보간된 위치를 실제 타겟으로 설정
	TargetLocation = InterpolatedTargetLocation;
	AquaticMovementComponent->SetTargetLocation(TargetLocation, MinTargetDistance);
	
	// 블랙보드의 TargetLocation을 보간된 값으로 업데이트 (추적 시에는 플레이어 위치)
	if (EnhancedAIController && EnhancedAIController->GetBlackboardComponent())
	{
		EnhancedAIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", InterpolatedTargetLocation);
	}
	
	// 추적 속도 설정
	AquaticMovementComponent->MaxSpeed = StatComponent->MoveSpeed * ChasingMovementSpeedMultiplier;
	AquaticMovementComponent->TurnSpeed = RotationInterpSpeed * ChasingRotationSpeedMultiplier * 30.0f;
	
#if WITH_EDITOR
	if (bDrawDebugLine)
	{
		// 보간된 타겟 위치를 빨간색으로 표시
		DrawDebugSphere(GetWorld(), InterpolatedTargetLocation, 40.0f, 8, FColor::Red, false, 0.1f);
		// 플레이어 위치와의 선
		DrawDebugLine(GetWorld(), InterpolatedTargetLocation, PlayerLocation, FColor::Red, false, 0.1f, 0, 2.0f);
	}
#endif
	
	/*
	   [DEPRECATED] 기존 NavMesh 기반 추적 시스템 - AquaticMovementComponent로 대체됨
	   const FVector CurrentLocation = GetActorLocation();

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
	        //경로가 없는 경우 직접 플레이어 방향으로 (기존 로직)
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
	*/
}

void ABoss::StartTurn()
{
	/*
	   [DEPRECATED] 기존 회전 시스템 - 새로운 AquaticSteeringMovementComponent로 대체 예정
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
	   		End,                                                       검사할 위치
	   		FQuat::Identity,                                                      회전 없음
	   		ECC_Visibility,                                                       충돌 채널
	   		FCollisionShape::MakeSphere(SphereOverlapRadius),    Sphere 크기
	   		Params                                                                충돌 파라미터
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
*/
}

void ABoss::ReturnToNavMeshArea()
{
	/*
	   [DEPRECATED] 기존 NavMesh 복귀 시스템 - 새로운 AquaticSteeringMovementComponent로 대체 예정
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
	*/
}

void ABoss::PerformTurn(const float& InDeltaTime)
{
	/*
	   [DEPRECATED] 기존 회전 수행 시스템 - 새로운 AquaticSteeringMovementComponent로 대체 예정
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
	*/
}

bool ABoss::HasObstacleAhead()
{
	// AquaticMovementComponent가 자체적으로 장애물 회피를 처리하므로
	// 이 함수는 더 이상 필요하지 않지만, 호환성을 위해 유지
	// 항상 false를 반환하여 AI가 장애물 체크를 건너뛰도록 함
	return false;
}

bool ABoss::IsLocationOnNavMesh(const FVector& InLocation) const
{
	// [DEPRECATED] - NavMesh는 더 이상 사용되지 않습니다.
	// 호환성을 위해 항상 true를 반환합니다.

	/*
	   [DEPRECATED] 기존 NavMesh 검증 시스템
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
	*/
	return true;    //임시로 true 반환
}
#pragma endregion

#pragma region 보스 상태 관련 함수
void ABoss::SetCharacterMovementSetting(const float& InBrakingDecelerationSwimming, const float& InMaxSwimSpeed)
{
	if (!IsValid(GetCharacterMovement())) return;

	if (InBrakingDecelerationSwimming < 0.0f || InMaxSwimSpeed <= 0.0f) return;

	GetCharacterMovement()->BrakingDecelerationSwimming = InBrakingDecelerationSwimming;
	GetCharacterMovement()->MaxSwimSpeed = InMaxSwimSpeed;
	
	// AquaticMovementComponent 설정도 동기화
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->BrakingDeceleration = InBrakingDecelerationSwimming;
		AquaticMovementComponent->MaxSpeed = InMaxSwimSpeed;
	}
}

void ABoss::InitCharacterMovementSetting()
{
	if (!IsValid(GetCharacterMovement())) return;
	
	GetCharacterMovement()->BrakingDecelerationSwimming = OriginDeceleration;
	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
	
	// AquaticMovementComponent 설정도 초기화
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->BrakingDeceleration = OriginDeceleration;
		AquaticMovementComponent->MaxSpeed = StatComponent->GetMoveSpeed();
	}
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

FVector ABoss::GetNextPatrolPoint()
{
	/*
	   [DEPRECATED] 기존 NavMesh 기반 순찰 시스템 - 새로운 AquaticSteeringMovementComponent로 대체 예정
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
	*/
	return GetActorLocation();
}
#pragma endregion