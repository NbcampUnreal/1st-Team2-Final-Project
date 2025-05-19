#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EBossState.h"
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
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	AttackCollision->SetCapsuleHalfHeight(80.0f);
	AttackCollision->SetCapsuleRadius(80.0f);
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->ComponentTags.Add(TEXT("Attack Collision"));

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>("Camera Controller Component");
	
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

void ABoss::LaunchPlayer(AUnderwaterCharacter* Player, float& Power)
{
	// 플레이어를 밀치는 로직
	const FVector PushDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const float PushStrength = Power; // 밀치는 힘의 크기 -> 변수화 필요할 것 같은데 일단 고민
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

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
                        AActor* DamageCauser)
{
	// 사망 상태면 얼리 리턴
	if (BlackboardComponent->GetValueAsEnum(BossStateKey) == static_cast<uint8>(EBossState::Death)) return 0.0f;

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 부위 타격 정보
	// @TODO : 맞은 부위에 따라 추가 데미지 혹은 출혈 이펙트 출력
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamage = static_cast<const FPointDamageEvent*>(&DamageEvent);
		
		FHitResult HitResult = PointDamage->HitInfo;
		if (HitResult.BoneName != NAME_None)
		{
			LOG(TEXT("Hit Bone: %s"), *HitResult.BoneName.ToString());
		}

		if (HitResult.PhysMaterial.IsValid())
		{
			FString CollisionName = HitResult.PhysMaterial->GetName();
			LOG(TEXT("Hit Collision: %s"), *CollisionName);
		}

		if (HitResult.PhysicsObjectOwner.IsValid())
		{
			FName RegionName = *HitResult.PhysicsObjectOwner->GetName();
			LOG(TEXT("%s"), *RegionName.ToString());			
		}

		if (HitResult.ImpactPoint != FVector::ZeroVector)
		{
			LOG(TEXT("Damage Location: %s"), *HitResult.ImpactPoint.ToString());
		}
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
	// 사망 시 가라앉는 연출
	GetCharacterMovement()->GravityScale = 0.1f;

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

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void ABoss::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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
AUnderwaterCharacter* ABoss::GetTarget()
{
	if (!IsValid(TargetPlayer)) return nullptr;
	
	return TargetPlayer;
}

void ABoss::SetTarget(AUnderwaterCharacter* Target)
{
	if (!IsValid(Target)) return;
	
	TargetPlayer = Target;
}

void ABoss::InitTarget()
{
	TargetPlayer = nullptr;
}

void ABoss::SetLastDetectedLocation(const FVector& InLastDetectedLocation)
{
	LastDetectedLocation = InLastDetectedLocation;
}

FVector ABoss::GetTargetPointLocation()
{
	if (!PatrolPoints.IsValidIndex(CurrentPatrolPointIndex)) return FVector::ZeroVector;
	
	return PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
}

bool ABoss::GetIsAttackCollisionOverlappedPlayer()
{
	return bIsAttackCollisionOverlappedPlayer;
}

UCameraControllerComponent* ABoss::GetCameraControllerComponent() const
{
	return CameraControllerComponent;
}

AActor* ABoss::GetTargetPoint()
{
	if (!PatrolPoints.IsValidIndex(CurrentPatrolPointIndex)) return nullptr;
	
	return PatrolPoints[CurrentPatrolPointIndex];
}
#pragma endregion