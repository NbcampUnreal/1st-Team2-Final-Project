#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
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
#include "Perception/AISense_Damage.h"

const FName ABoss::BossStateKey = "BossState";

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
	Acceleration = 2.f;
	
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

	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapBegin);
	AttackCollision->OnComponentEndOverlap.AddDynamic(this, &ABoss::OnAttackCollisionOverlapEnd);

	GetCharacterMovement()->MaxFlySpeed = StatComponent->GetMoveSpeed();
}

void ABoss::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABoss, BossState);
}

void ABoss::SetBossState(EBossState State)
{
	if (!HasAuthority()) return;

	BossState = State;
	BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));
}

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

void ABoss::MoveForward(const float& InDeltaTime)
{
	// 목표 속도까지 부드럽게 가속
	CurrentMoveSpeed = FMath::FInterpTo(CurrentMoveSpeed, StatComponent->GetMoveSpeed(), InDeltaTime, Acceleration);
	
	// 실제 이동
	const FVector MoveDelta = GetActorForwardVector() * CurrentMoveSpeed * InDeltaTime;

	// 충돌 적용
	SetActorLocation(GetActorLocation() + MoveDelta, true);
}

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


void ABoss::RotationToTarget(AActor* Target)
{
	if (!IsValid(Target)) return;

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (Target->GetActorLocation() - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
	
	SetActorRotation(NewRotation);
}

void ABoss::RotationToTarget(const FVector& TargetLocation)
{
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (TargetLocation - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
	
	SetActorRotation(NewRotation);
}

void ABoss::Attack()
{
	const uint8 AttackType = FMath::RandRange(0, NormalAttackAnimations.Num() - 1);
	
	if (IsValid(NormalAttackAnimations[AttackType]))
	{
		M_PlayAnimation(NormalAttackAnimations[AttackType]);
	}
}

void ABoss::OnAttackEnded()
{
	AttackedPlayers.Empty();
}

void ABoss::AddPatrolPoint()
{
	if (CurrentPatrolPointIndex >= PatrolPoints.Num())
	{
		CurrentPatrolPointIndex = 0;
	}
	else
	{
		++CurrentPatrolPointIndex;
	}
}

void ABoss::SetMoveSpeed(const float& SpeedMultiplier)
{
	GetCharacterMovement()->MaxFlySpeed = StatComponent->MoveSpeed * SpeedMultiplier;
}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
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

	for (uint8 i = 0; i < MaxTries; ++i)
	{
		bool bFound = NavSys->GetRandomReachablePointInRadius(GetActorLocation(), MaxPatrolDistance, RandomNavLocation);

		if (bFound)
		{
			const FVector CurrentLocation = GetActorLocation();
			const FVector TargetLocation = RandomNavLocation.Location;

			if (FVector::Distance(CurrentLocation, TargetLocation) > MinPatrolDistance)
			{
				return TargetLocation;
			}
		}	
	}

	return GetActorLocation();
}

#pragma region Getter, Setter

const FVector ABoss::GetTargetPointLocation() const
{
	if (!PatrolPoints.IsValidIndex(CurrentPatrolPointIndex)) return FVector::ZeroVector;
	
	return PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
}

AActor* ABoss::GetTargetPoint()
{
	if (!PatrolPoints.IsValidIndex(CurrentPatrolPointIndex)) return nullptr;
	
	return PatrolPoints[CurrentPatrolPointIndex];
}
#pragma endregion