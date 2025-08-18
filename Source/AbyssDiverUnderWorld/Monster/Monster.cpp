// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"
#include "Character/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"
#include "AnimNodes/AquaticMovementComponent.h"

const FName AMonster::MonsterStateKey = "MonsterState";
const FName AMonster::InvestigateLocationKey = "InvestigateLocation";
const FName AMonster::PatrolLocationKey = "PatrolLocation";
const FName AMonster::TargetActorKey = "TargetActor";

AMonster::AMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize
	BlackboardComponent = nullptr;
	AIController = nullptr;
	AnimInstance = nullptr;
	TargetActor = nullptr;
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
	// Set Collision Channel == Monster
	GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel3);

	//RadarReturnComponent->FactionTags.Init(TEXT("Hostile"), 1);

	// 새로운 물리 기반 수중 이동 컴포넌트 초기화
	AquaticMovementComponent = CreateDefaultSubobject<UAquaticMovementComponent>("Aquatic Movement Component");
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	// AquaticMovementComponent 초기화
	if (AquaticMovementComponent)
	{
		AquaticMovementComponent->InitComponent(this);

		// Boss의 기본 이동 속도 설정 (컴포넌트 변수를 무시하고 CharcterMovementComponent의 값을 사용하려면 주석 제거)
		//AquaticMovementComponent->MaxSpeed = GetCharacterMovement()->MaxSwimSpeed;
		//AquaticMovementComponent->Acceleration = 200.0f;
		//AquaticMovementComponent->TurnSpeed = RotationInterpSpeed * 30.0f; // 도/초 단위로 변환
		//AquaticMovementComponent->BrakingDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;
	}

	CurrentMoveSpeed = StatComponent->MoveSpeed;
	AnimInstance = GetMesh()->GetAnimInstance();
	AIController = Cast<AMonsterAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
		SetMonsterState(EMonsterState::Patrol);

		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsVector("TargetLocation", GetActorLocation());
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

	DOREPLIFETIME(AMonster, MonsterState);
	DOREPLIFETIME(AMonster, bIsChasing);
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
		AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer") != nullptr;

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
				AIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
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
				AIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
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
	if (AIController && AIController->GetBlackboardComponent())
	{
		AIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", DesiredTargetLocation);
	}

	DrawDebugSphere(GetWorld(), DesiredTargetLocation, 50.0f, 12, FColor::Red, false, 3.0f, 0, 5.0f);
	DrawDebugLine(GetWorld(), CurrentLocation, DesiredTargetLocation, FColor::Red, false, 3.0f, 0, 3.0f);
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
		AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer") != nullptr;

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
		if (AIController && AIController->GetBlackboardComponent())
		{
			AIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", InterpolatedTargetLocation);
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

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
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
		AIController->GetBlackboardComponent()->SetValueAsVector("TargetLocation", InterpolatedTargetLocation);
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
		FVector BloodLoc = GetActorLocation() + FVector(0, 0, 20.f);
		FRotator BloodRot = GetActorRotation();
		M_SpawnBloodEffect(BloodLoc, BloodRot);
		
		AActor* InstigatorPlayer = IsValid(EventInstigator) ? EventInstigator->GetPawn() : nullptr;
		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
			// Delegate Broadcasts for Achievements
			OnMonsterDead.Broadcast(DamageCauser, this);

			// Disable aggro when dead
			if (TargetActor)
			{
				ForceRemoveDetection(TargetActor);
			}
			
			if (InstigatorPlayer)
			{
				ForceRemoveDetection(InstigatorPlayer);
			}
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
			if (MonsterState != EMonsterState::Chase && MonsterState != EMonsterState::Flee)
			{
				AddDetection(InstigatorPlayer);
				SetMonsterState(EMonsterState::Chase);
			}
		}
	}
	return Damage;
}

void AMonster::OnDeath()
{
	if (MonsterSoundComponent)
	{
		// Stop Existing LoopSound
		MonsterSoundComponent->M_StopAllLoopSound();
	}

	UnPossessAI();
	M_OnDeath();
	MonsterRaderOff();

	FTimerHandle DestroyTimerHandle;
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DestroyTimerHandle, this, &AMonster::DelayDestroyed, 30.0f, false
		);
	}

	SetMonsterState(EMonsterState::Death);
}

void AMonster::M_OnDeath_Implementation()
{
	GetCharacterMovement()->StopMovementImmediately();
	
	if (IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(1.0f);
	}

	// Applying the Physics Asset Physics Engine
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AMonster::ApplyPhysicsSimulation, 0.5f, false);
}

void AMonster::ApplyPhysicsSimulation()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetAttackHitComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetSimulatePhysics(true);
}

void AMonster::PlayAttackMontage()
{
	if (!HasAuthority()) return;

	if (!AnimInstance) return;

	// If any montage is playing, prevent duplicate playback
	if (AnimInstance->IsAnyMontagePlaying()) return;
	
	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);

	UAnimMontage* SelectedMontage = AttackAnimations[AttackType];

	if (IsValid(SelectedMontage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Playing Attack Montage: %s"), *SelectedMontage->GetName());
		M_PlayMontage(SelectedMontage);
		CurrentAttackAnim = SelectedMontage;
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

void AMonster::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	if (!HasAuthority()) return;
	if (!IsValid(this)) return;
	if (AIController == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("TotalExposedTime : %.2f"), TotalExposedTime);
	switch (MonsterState)
	{
	case EMonsterState::Patrol:
		SetMonsterState(EMonsterState::Detected);
		break;

	case EMonsterState::Detected:
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			AddDetection(PlayerActor);
			SetMonsterState(EMonsterState::Chase);
		}
		else
		{
			SetMonsterState(EMonsterState::Investigate);
		}
		break;

	case EMonsterState::Investigate:
	{
		if (AIController)
		{
			if (BlackboardComponent)
			{
				BlackboardComponent->SetValueAsVector(InvestigateLocationKey, PlayerLocation);
			}
		}
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			SetMonsterState(EMonsterState::Chase);
			AddDetection(PlayerActor);
		}
		else if (TotalExposedTime < ChaseTriggerTime)
		{
			if (TargetActor == nullptr)
			{
				SetMonsterState(EMonsterState::Investigate);
			}
			else
			{
				SetMonsterState(EMonsterState::Chase);
			}
		}
		break;
	}
	case EMonsterState::Chase:
		if (TargetActor != PlayerActor)
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

	int32& Count = DetectionRefCounts.FindOrAdd(Actor);
	Count++;

	UE_LOG(LogTemp, Log, TEXT("[%s] AddDetection : %s | Count: %d"),
		*GetName(),
		*Actor->GetName(),
		Count
	);

	// If Target is empty, set
	if (TargetActor == nullptr || !IsValid(TargetActor))
	{
		TargetActor = Actor;

		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (Player)
		{
			Player->OnTargeted(this);
		}

		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
		}
	}
}

void AMonster::RemoveDetection(AActor* Actor)
{
	if (!IsValid(this)) return;

	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RemoveDetection : Invalid Player! Force remove."), *GetName());
		DetectionRefCounts.Remove(Actor);

		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (Player)
		{
			Player->OnUntargeted(this);
		}

		return;
	}

	int32* CountPtr = DetectionRefCounts.Find(Actor);
	if (!CountPtr) return;

	(*CountPtr)--;

	UE_LOG(LogTemp, Log, TEXT("[%s] RemoveDetection : %s | New Count: %d"),
		*GetName(),
		*Actor->GetName(),
		*CountPtr
	);

	if (*CountPtr <= 0)
	{
		DetectionRefCounts.Remove(Actor);

		if (TargetActor == Actor)
		{
			TargetActor = nullptr;

			AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
			if (Player)
			{
				Player->OnUntargeted(this);
			}

			if (BlackboardComponent)
			{
				BlackboardComponent->ClearValue(TargetActorKey);
			}

			// Alternate targeting
			for (const auto& Pair : DetectionRefCounts)
			{
				if (IsValid(Pair.Key))
				{
					TargetActor = Pair.Key;

					AUnderwaterCharacter* NextAgrroPlayer = Cast<AUnderwaterCharacter>(Pair.Key);
					if (NextAgrroPlayer)
					{
						NextAgrroPlayer->OnTargeted(this);
					}

					if (BlackboardComponent)
					{
						SetMonsterState(EMonsterState::Chase);
						BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
					}
					UE_LOG(LogTemp, Log, TEXT("[%s] New TargetActor: %s"), *GetName(), *TargetActor->GetName());

					break;
				}
			}
		}
	}
}

/**
Quick fix : Because it is a 3D space, RefCount sometimes goes up abnormally when AddDetection is perceived by Perception.
Therefore, when the TargetActor is lost for more than LoseRadius && MaxAge time after Perception.
Methods to bring down abnormally high RefCount at once 
**/
void AMonster::ForceRemoveDetection(AActor* Actor)
{
	if (!IsValid(this)) return;

	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RemoveDetection : Invalid Player! Force remove."), *GetName());
		DetectionRefCounts.Remove(Actor);

		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (Player)
		{
			Player->OnUntargeted(this);
		}

		return;
	}

	int32* CountPtr = DetectionRefCounts.Find(Actor);
	if (CountPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ForceRemoveDetection] Actor: %s, OldCount: %d -> 0"),
			*Actor->GetName(), *CountPtr);

		*CountPtr = 0;
		DetectionRefCounts.Remove(Actor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ForceRemoveDetection] Actor: %s not found in DetectionRefCounts"),
			*Actor->GetName());
	}

	if (TargetActor == Actor)
	{
		TargetActor = nullptr;
		
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (Player)
		{
			Player->OnUntargeted(this);
		}

		if (BlackboardComponent)
		{
			BlackboardComponent->ClearValue(TargetActorKey);
		}

		// Alternate targeting
		for (const auto& Pair : DetectionRefCounts)
		{
			if (IsValid(Pair.Key))
			{
				TargetActor = Pair.Key;
				
				AUnderwaterCharacter* NextAgrroPlayer = Cast<AUnderwaterCharacter>(Pair.Key);
				if (NextAgrroPlayer)
				{
					NextAgrroPlayer->OnTargeted(this);
				}

				if (BlackboardComponent)
				{
					SetMonsterState(EMonsterState::Chase);
					BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
				}
				break;
			}
		}
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

void AMonster::SetMonsterState(EMonsterState NewState)
{
	if (!HasAuthority()) return;

	if (MonsterState == NewState) return;

	if (GetController() == nullptr) return;

	if (MonsterSoundComponent)
	{
		// Stop Existing LoopSound
		MonsterSoundComponent->S_StopAllLoopSound();
	}

	FString StateToString = StaticEnum<EMonsterState>()->GetNameStringByValue((int64)MonsterState);
	FString NewStateToString = StaticEnum<EMonsterState>()->GetNameStringByValue((int64)NewState);
	UE_LOG(LogTemp, Warning, TEXT("MonsterState changed: %s -> %s"), *StateToString, *NewStateToString);

	MonsterState = NewState;

	if (UBlackboardComponent* BB = Cast<AAIController>(GetController())->GetBlackboardComponent())
	{
		BB->SetValueAsEnum(MonsterStateKey, static_cast<uint8>(NewState));
	}

	switch (NewState)
	{
	case EMonsterState::Detected:
		if (IsValid(DetectedAnimations))
		{
			M_PlayMontage(DetectedAnimations);
		}
		break;

	case EMonsterState::Chase:
		SetMaxSwimSpeed(ChaseSpeed);
		MonsterSoundComponent->S_PlayChaseLoopSound();
		bIsChasing = true;

		if (BlackboardComponent)
		{
			BlackboardComponent->ClearValue(InvestigateLocationKey);
			BlackboardComponent->ClearValue(PatrolLocationKey);
		}
		break;

	case EMonsterState::Patrol:
		SetMaxSwimSpeed(PatrolSpeed);
		MonsterSoundComponent->S_PlayPatrolLoopSound();
		if (TargetActor != nullptr)
		{
			ForceRemoveDetection(TargetActor);
		}

		if (BlackboardComponent)
		{
			BlackboardComponent->ClearValue(InvestigateLocationKey);
		}
		
		bIsChasing = false;
		break;

	case EMonsterState::Investigate:
		SetMaxSwimSpeed(InvestigateSpeed);
		bIsChasing = false;

		if (BlackboardComponent)
		{
			BlackboardComponent->ClearValue(PatrolLocationKey);
		}

		break;

	case EMonsterState::Flee:
		SetMaxSwimSpeed(FleeSpeed);
		MonsterSoundComponent->S_PlayFleeLoopSound();
		bIsChasing = false;

	default:
		break;
	}
}

void AMonster::SetMaxSwimSpeed(float Speed)
{
	GetCharacterMovement()->MaxSwimSpeed = Speed;
}

int32 AMonster::GetDetectionCount() const
{
	int32 ValidCount = 0;

	for (const auto& Elem : DetectionRefCounts)
	{
		if (Elem.Value > 0 && IsValid(Elem.Key))
		{
			++ValidCount;
		}
	}

	return ValidCount;
}



