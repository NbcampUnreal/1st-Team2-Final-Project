#include "Monster/Boss/Boss.h"
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
#include "AnimNodes/AquaticMovementComponent.h"

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

	//// 새로운 물리 기반 수중 이동 컴포넌트 초기화
	//AquaticMovementComponent = CreateDefaultSubobject<UAquaticMovementComponent>("Aquatic Movement Component");

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

void ABoss::SetNewTargetLocation()
{
	Super::SetNewTargetLocation();
}

void ABoss::PerformNormalMovement(const float& InDeltaTime)
{
	Super::PerformNormalMovement(InDeltaTime);

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
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		BossState = State;
		AIC->GetBlackboardComponent()->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));	
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
	return GetActorLocation();
}
#pragma endregion