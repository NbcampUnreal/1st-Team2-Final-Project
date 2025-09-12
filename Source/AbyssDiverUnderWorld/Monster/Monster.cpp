#include "Monster/Monster.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/Components/TickControlComponent.h"
#include "Monster/EPerceptionType.h"
#include "Monster/Effect/CameraControllerComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Character/StatComponent.h"

#include "Container/BlackboardKeys.h"
#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

AMonster::AMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize
	bIsAttackCollisionOverlappedPlayer = false;
	BlackboardComponent = nullptr;
	AIController = nullptr;
	AnimInstance = nullptr;
	TargetPlayer = nullptr;
	ChaseTriggerTime = 1.8f;
	ChaseSpeed = 400.0f;
	PatrolSpeed = 200.0f;
	InvestigateSpeed = 300.0f;
	bIsChasing = false;

	bUseControllerRotationYaw = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	MonsterSoundComponent = CreateDefaultSubobject<UMonsterSoundComponent>(TEXT("MonsterSoundComponent"));

	// Set Default PossessSetting
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// Set Collision Channel == Monster(ECC_GameTraceChannel3)
	GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel3);

	AttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	AttackCollision->SetCapsuleHalfHeight(80.0f);
	AttackCollision->SetCapsuleRadius(80.0f);
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->ComponentTags.Add(TEXT("Attack Collision"));

	// 공격 시 카메라 쉐이킹을 위한 컴포넌트
	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>("Camera Controller Component");

	//RadarReturnComponent->FactionTags.Init(TEXT("Hostile"), 1);

	// 새로운 물리 기반 수중 이동 컴포넌트 초기화
	AquaticMovementComponent = CreateDefaultSubobject<UAquaticMovementComponent>("Aquatic Movement Component");

	// 틱 최적화용 컴포넌트 초기화
	TickControlComponent = CreateDefaultSubobject<UTickControlComponent>(TEXT("Tick Control Component"));

}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	TickControlComponent->RegisterComponent(GetMesh());
	TickControlComponent->AddIgnoreActor(this);

	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->InitComponent(this);
		TickControlComponent->RegisterComponent(AquaticMovementComponent);
	}

	CurrentMoveSpeed = StatComponent->MoveSpeed;
	AnimInstance = GetMesh()->GetAnimInstance();
	AIController = Cast<AMonsterAIController>(GetController());

	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AMonster::OnAttackCollisionOverlapBegin);
	AttackCollision->OnComponentEndOverlap.AddDynamic(this, &AMonster::OnAttackCollisionOverlapEnd);

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsVector(BlackboardKeys::TargetLocationKey, GetActorLocation());
			ApplyMonsterStateChange(EMonsterState::Idle);
		}
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
	}

	Params.AddIgnoredActor(this);
}

void AMonster::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMonster::OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	bIsAttackCollisionOverlappedPlayer = true;
}

void AMonster::OnAttackCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 공격 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	bIsAttackCollisionOverlappedPlayer = false;
}

void AMonster::SetNewTargetLocation()
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
	const bool bHasTargetPlayer = AIController &&
		AIController->GetBlackboardComponent() &&
		AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey) != nullptr;

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
			if (AIController && AIController->GetBlackboardComponent())
			{
				AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, DesiredTargetLocation);
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
			if (AIController && AIController->GetBlackboardComponent())
			{
				AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, DesiredTargetLocation);
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
		DrawDebugLine(GetWorld(), CurrentLocation, DesiredTargetLocation, FColor::Red, false, 3.0f, 0, 3.0f);
	}
#endif

	// 블랙보드의 TargetLocation 업데이트
	if (AIController && AIController->GetBlackboardComponent())
	{
		AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, DesiredTargetLocation);
	}

	//DrawDebugSphere(GetWorld(), DesiredTargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
	
}

void AMonster::PerformNormalMovement(const float& InDeltaTime)
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
	const bool bHasTargetPlayer = AIController &&
		AIController->GetBlackboardComponent() &&
		AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey) != nullptr;

	if (!bHasTargetPlayer)
	{
		// 목표점에 가까워지면 미리 새 목표 설정 (멈추지 않고 계속 이동)
		if (DistanceToDesiredTarget < 500 || DesiredTargetLocation.IsZero())
		{
			SetNewTargetLocation();

			if (MonsterState == EMonsterState::Investigate)
			{
				ApplyMonsterStateChange(EMonsterState::Patrol);
			}
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
		if (AIController && AIController->GetBlackboardComponent())
		{
			AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, InterpolatedTargetLocation);
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

void AMonster::PerformChasing(const float& InDeltaTime)
{
	if (!AquaticMovementComponent)
	{
		LOG(TEXT("PerformChasing: AquaticMovementComponent is null"));
		return;
	}

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
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
	if (AIController && AIController->GetBlackboardComponent())
	{
		AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, InterpolatedTargetLocation);
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

void AMonster::Attack()
{
	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);

	if (!AnimInstance) return;

	if (AnimInstance->IsAnyMontagePlaying()) return;

	if (IsValid(AttackAnimations[AttackType]))
	{
		M_PlayMontage(AttackAnimations[AttackType]);
	}

	bIsAttacking = true;
}

void AMonster::OnAttackEnded()
{
	bIsAttacking = false;
	AttackedPlayers.Empty();
}

void AMonster::M_PlayMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void AMonster::M_SpawnBloodEffect_Implementation(FVector Location, FRotator Rotation)
{
	if (BloodEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodEffect, Location, Rotation);
	}
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsValid(StatComponent))
	{
		FVector BloodLoc = GetMesh()->GetComponentLocation() + FVector(0, 0, 20.f);
		FRotator BloodRot = GetActorRotation();
		M_SpawnBloodEffect(BloodLoc, BloodRot);

		
		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
			// Delegate Broadcasts for Achievements
			OnMonsterDead.Broadcast(DamageCauser, this);
		}
		else
		{
			if (IsValid(HitReactAnimations))
			{
				M_PlayMontage(HitReactAnimations);
			}
			else if (MonsterSoundComponent)
			{
				MonsterSoundComponent->S_PlayHitReactSound();
			}
			
			// If aggro is not on TargetPlayer, set aggro
			AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController());
			if (AIC == nullptr)
			{
				LOGV(Error, TEXT("AIController Is not valid"));
				return Damage;
			}

			AActor* InstigatorPlayer = IsValid(EventInstigator) ? EventInstigator->GetPawn() : nullptr;
			if (InstigatorPlayer == nullptr)
			{
				LOGV(Warning, TEXT("InstigatorPlayer is not valid, So Detection Is Failed (%s)"), *GetName());
				return Damage;
			}

			// 시야 범위나 청각 범위 중 넓은 범위 내에서 데미지를 받았을 경우 어그로 Set
			float RecogniztionRange = FMath::Max3(AIC->GetSightRadius(), AIC->GetLoseSightRadius(), AIC->GetHearingRadius());
			if ((InstigatorPlayer->GetActorLocation() - GetActorLocation()).Length() <= RecogniztionRange)
			{
				AddDetection(InstigatorPlayer);
			}
		}
	}

	return Damage;
}

void AMonster::OnDeath()
{
	ApplyMonsterStateChange(EMonsterState::Death);

	if (MonsterSoundComponent)
	{
		// 모든 사운드 해제
		MonsterSoundComponent->M_StopAllLoopSound();
	}

	// 사망하면 모든 어그로 해제
	ForceRemoveDetectedPlayers();

	// 렉돌 활성화 및 AquaticMovementComponent Tick 비활성화
	HandleSetting_OnDeath();

	// 모든 몽타주 재생 정지
	M_OnDeath();

	UnPossessAI();
	MonsterRaderOff();

	FTimerHandle DestroyTimerHandle;
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DestroyTimerHandle, this, &AMonster::DelayDestroyed, 30.0f, false
		);
	}
}

void AMonster::M_OnDeath_Implementation()
{
	if (IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(1.0f);
	}
}

void AMonster::HandleSetting_OnDeath()
{
	// 죽었을 때 Tick 비활성화
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->SetComponentTickEnabled(false);
	}
	ApplyPhysicsSimulation();
}

void AMonster::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 사망 상태면 얼리 리턴
	if (MonsterState == EMonsterState::Death) return;

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

void AMonster::ApplyPhysicsSimulation()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetEnableGravity(true);
		MeshComp->SetSimulatePhysics(true);
	}
}

void AMonster::UnPossessAI()
{
	if (IsValid(AIController))
	{
		AIController->StopMovement();
		AIController->UnPossess();
	}
}

// 랜턴이 비춰지는 시간에 따른 상태변화 함수
void AMonster::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	if (!HasAuthority()) return;
	if (!IsValid(this)) return;
	if (AIController == nullptr) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(PlayerActor);
	if (Player == nullptr)
	{
		LOGV(Error, TEXT("NotifyLightExposure: Player is not valid"));
		return;
	}

	if (Player->IsHideInSeaweed())
	{
		LOGV(Log, TEXT("NotifyLightExposure: Player is hiding in seaweed, ignoring light exposure"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TotalExposedTime : %.2f"), TotalExposedTime);
	switch (MonsterState)
	{
	case EMonsterState::Patrol:
		// 조명 비추자 마자 Detected로 상태 변화
		ApplyMonsterStateChange(EMonsterState::Detected);
		break;

	case EMonsterState::Detected:
		// 사실상 찰나임 (Detected Animation을 재생하기 위한 상태)
		ApplyMonsterStateChange(EMonsterState::Investigate);
		break;

	case EMonsterState::Investigate:
	{
		// 조명을 비추는 곳(플레이어 위치)을 최종목표지점으로 업데이트
		DesiredTargetLocation = PlayerLocation;
		
		// 일정 시간 이상 조명이 비춰질 경우 실행
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			ApplyMonsterStateChange(EMonsterState::Chase);
			AddDetection(PlayerActor);
		}
		break;
	}
	case EMonsterState::Chase:
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			AddDetection(PlayerActor);
		}
		break;

	case EMonsterState::Flee:
		break;
	
	default:
		break;
	}
}

void AMonster::AddDetection(AActor* Actor)
{
	if (!IsValid(Actor) || !IsValid(this)) return;
	if (!HasAuthority()) return;

	if (GetMonsterState() == EMonsterState::Death)
	{
		return;
	}

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!Player) return;

	if (!DetectedPlayers.Contains(Actor))
	{
		DetectedPlayers.Add(Actor);

		UE_LOG(LogTemp, Log, TEXT("[%s] AddDetection : %s | ArraySize: %d"),
			*GetName(),
			*Actor->GetName(),
			DetectedPlayers.Num()
		);

		Player->OnTargeted(this);
	}
	else return;

	// 만약 TargetPlayer가 없으면 TargetPlayer를 해당 Actor(Player)로 설정.
	if (!TargetPlayer.IsValid())
	{
		TargetPlayer = Player;

		if (BlackboardComponent)
		{
			//BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, TargetPlayer.Get());
			SetTarget(TargetPlayer.Get());
			ApplyMonsterStateChange(EMonsterState::Chase);
		}
	}
}

void AMonster::RemoveDetection(AActor* Actor)
{
	if (!IsValid(this) || !IsValid(Actor)) return;
	if (!HasAuthority()) return;

	// DetectedPlayers에 Actor가 없으면 return;
	if (!DetectedPlayers.Remove(Actor)) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] RemoveDetection : %s | ArraySize: %d"),
		*GetName(),
		*Actor->GetName(),
		DetectedPlayers.Num())

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!Player)
	{
		LOG(TEXT("Player가 유효하지 않음"));
		return;
	}

	Player->OnUntargeted(this);

	// 만약 현재 설정된 TargetPlayer가 빠졌다면 다른 타겟 지정해야함
	const bool bWasTarget = (TargetPlayer.Get() == Actor);

	if (bWasTarget)
	{
		// 우선 TargetPlayer 비움.
		//InitTarget();
		TargetPlayer.Reset();

		if (BlackboardComponent)
		{
			//BlackboardComponent->ClearValue(BlackboardKeys::TargetPlayerKey);
			SetTarget(nullptr);
		}

		// 후보가 없으면 바로 Patrol (루프 스킵)
		if (DetectedPlayers.Num() == 0)
		{
			ApplyMonsterStateChange(EMonsterState::Patrol);
			return;
		}

		// TSet 순회하여 요소(Player)가 남아있으면 해당 플레이어를 TargetPlayer로 지정
		for (const TWeakObjectPtr<AActor>& Elem : DetectedPlayers)
		{
			if (AActor* NewTarget = Elem.Get())
			{
				AUnderwaterCharacter* DetectedPlayer = Cast<AUnderwaterCharacter>(NewTarget);
				
				if (DetectedPlayer)
				{
					TargetPlayer = DetectedPlayer;
				}

				if (BlackboardComponent)
				{
					//BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, TargetPlayer.Get());
					SetTarget(TargetPlayer.Get());
				}
				return;
			}
		}

		// 후보가 있지만 전부 무효일 수도 있음 (TWeakObjectPtr) -> Patrol로 상태 설정
		ApplyMonsterStateChange(EMonsterState::Patrol);
	}
}

void AMonster::ForceRemoveDetectedPlayers()
{
	if (!IsValid(this)) return;
	if (!HasAuthority()) return;

	// 이미 비어있으면 return;
	if (DetectedPlayers.Num() == 0 && !TargetPlayer.IsValid())
	{
		return;
	}

	// 남아있는 DetectedPlayers 목록 캐싱 (컨테이너 안전)
	TArray<AActor*> TempArray;
	TempArray.Reserve(DetectedPlayers.Num());
	for (const TWeakObjectPtr<AActor>& Elem : DetectedPlayers)
	{
		if (AActor* TargetCandidate = Elem.Get())
		{
			TempArray.Add(TargetCandidate);
		}
	}

	for (AActor* TargetCandidate : TempArray)
	{
		if (AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TargetCandidate))
		{
			Player->OnUntargeted(this);
		}
	}

	DetectedPlayers.Reset();
	InitTarget();
	if (BlackboardComponent)
	{
		//BlackboardComponent->ClearValue(BlackboardKeys::TargetPlayerKey);
		SetTarget(nullptr);
	}
}

bool AMonster::IsAnimMontagePlaying() const
{
	if (AnimInstance)
	{
		return AnimInstance->Montage_IsPlaying(CurrentAttackAnim);
	}
	return false;
}

void AMonster::DelayDestroyed()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void AMonster::MonsterRaderOff()
{
	URadarReturn2DComponent* RaderComponent = Cast<URadarReturn2DComponent>(GetComponentByClass(URadarReturn2DComponent::StaticClass()));
	if (RaderComponent)
	{
		RaderComponent->SetAlwaysIgnore(true);
	}
}

void AMonster::LaunchPlayer(AUnderwaterCharacter* Player, const float& Power) const
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

void AMonster::SetMonsterState(EMonsterState NewState)
{
	if (!HasAuthority()) return;
	if (!AIController || MonsterState == NewState) return;
	
	MonsterState = NewState;

	if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
	{
		BB->SetValueAsEnum(BlackboardKeys::MonsterStateKey, static_cast<uint8>(NewState));
	}
}

void AMonster::ApplyMonsterStateChange(EMonsterState NewState)
{
	if (!HasAuthority()) return;
	if (MonsterState == NewState) return;
	if (!MonsterSoundComponent || !BlackboardComponent)	return;

	SetMonsterState(NewState);

	MonsterSoundComponent->S_StopAllLoopSound();

	switch (NewState)
	{
	case EMonsterState::Detected:

		if (IsValid(DetectedAnimations))
		{
			M_PlayMontage(DetectedAnimations);
		}
		break;

	case EMonsterState::Chase:

		bIsChasing = true;
		// 추적 속도는 이미 Monster::PerformChasing에서 변경하고 있음
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, true);
		MonsterSoundComponent->S_PlayChaseLoopSound();
		break;

	case EMonsterState::Patrol:

		bIsChasing = false;
		SetMaxSwimSpeed(PatrolSpeed);
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, false);
		MonsterSoundComponent->S_PlayPatrolLoopSound();
		ForceRemoveDetectedPlayers();
		break;

	case EMonsterState::Investigate:

		SetMaxSwimSpeed(InvestigateSpeed);
		break;

	case EMonsterState::Flee:

		bIsChasing = false;
		SetMaxSwimSpeed(FleeSpeed);
		MonsterSoundComponent->S_PlayFleeLoopSound();
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, false);
		break;

	default:
		break;
	}
}

void AMonster::InitCharacterMovementSetting()
{
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->BrakingDeceleration = OriginDeceleration;
		AquaticMovementComponent->MaxSpeed = StatComponent->GetMoveSpeed();
	}
}

void AMonster::SetMaxSwimSpeed(float Speed)
{
	AquaticMovementComponent->MaxSpeed = Speed;
}

void AMonster::SetTarget(AUnderwaterCharacter* Target)
{
	TargetPlayer = Target;
	AIController->GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Target);
}




