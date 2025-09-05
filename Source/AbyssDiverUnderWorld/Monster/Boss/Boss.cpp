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
#include "Engine/DamageEvents.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Net/UnrealNetwork.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Monster/EPerceptionType.h"
#include "Framework/ADPlayerController.h"
#include "Perception/AISense_Damage.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"
#include "Monster/Components/AquaticMovementComponent.h"

//const FName ABoss::BossStateKey = "BossState";

#pragma region 초기화 함수
ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;

	TargetPlayer = nullptr;
	LaunchPower = 1000.0f;
	MinPatrolDistance = 500.0f;
	MaxPatrolDistance = 1000.0f;
	AttackedCameraShakeScale = 1.0f;
	
	BossPhysicsType = EBossPhysicsType::None;
	DamagedLocation = FVector::ZeroVector;
	CachedSpawnLocation = FVector::ZeroVector;
	RotationInterpSpeed = 1.1f;

	bIsAttackInfinite = true;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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

	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
	OriginDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;
	CurrentMoveSpeed = StatComponent->MoveSpeed;

	SphereOverlapRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;

	Params.AddIgnoredActor(this);
}

//void ABoss::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(ABoss, BossState);
//}
#pragma endregion

//#pragma region 수중생물 AI 자체구현 함수
//
//void ABoss::SetNewTargetLocation()
//{
//	Super::SetNewTargetLocation();
//}
//
//void ABoss::PerformNormalMovement(const float& InDeltaTime)
//{
//	Super::PerformNormalMovement(InDeltaTime);
//}
//
//void ABoss::PerformChasing(const float& InDeltaTime)
//{
//	Super::PerformChasing(InDeltaTime);
//}
//
//#pragma endregion

#pragma region 보스 상태 관련 함수
//void ABoss::SetCharacterMovementSetting(const float& InBrakingDecelerationSwimming, const float& InMaxSwimSpeed)
//{
//	if (!IsValid(GetCharacterMovement())) return;
//
//	if (InBrakingDecelerationSwimming < 0.0f || InMaxSwimSpeed <= 0.0f) return;
//
//	GetCharacterMovement()->BrakingDecelerationSwimming = InBrakingDecelerationSwimming;
//	GetCharacterMovement()->MaxSwimSpeed = InMaxSwimSpeed;
//	
//	// AquaticMovementComponent 설정도 동기화
//	if (AquaticMovementComponent)
//	{
//		AquaticMovementComponent->BrakingDeceleration = InBrakingDecelerationSwimming;
//		AquaticMovementComponent->MaxSpeed = InMaxSwimSpeed;
//	}
//}

//void ABoss::InitCharacterMovementSetting()
//{
//	//if (!IsValid(GetCharacterMovement())) return;
//	//
//	//GetCharacterMovement()->BrakingDecelerationSwimming = OriginDeceleration;
//	//GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
//	
//	// AquaticMovementComponent 설정도 초기화
//	if (AquaticMovementComponent)
//	{
//		AquaticMovementComponent->BrakingDeceleration = OriginDeceleration;
//		AquaticMovementComponent->MaxSpeed = StatComponent->GetMoveSpeed();
//	}
//}

//void ABoss::SetBossState(EBossState State)
//{
//	if (!HasAuthority()) return;
//	
//	if (AAIController* AIC = Cast<AAIController>(GetController()))
//	{
//		//BossState = State;
//		//AIC->GetBlackboardComponent()->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));	
//	}
//}
#pragma endregion

#pragma region  TakeDamage, Death
float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
                        AActor* DamageCauser)
{
	// 사망 상태면 얼리 리턴
	if (MonsterState == EMonsterState::Death) return 0.0f;
	
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
	Super::OnDeath();

	/*M_OnDeath();
	
	DeathToRaderOff();

	if (IsValid(GetController()))
	{
		GetController()->StopMovement();
		GetController()->UnPossess();	
	}*/
}
#pragma endregion

#pragma region 보스 유틸 함수
//void ABoss::LaunchPlayer(AUnderwaterCharacter* Player, const float& Power) const
//{
//	// 플레이어를 밀치는 로직
//	const FVector PushDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
//	const float PushStrength = Power;
//	const FVector PushForce = PushDirection * PushStrength;
//	
//	// 물리 시뮬레이션이 아닌 경우 LaunchCharacter 사용
//	Player->LaunchCharacter(PushForce, false, false);
//
//	// 0.5초 후 캐릭터의 원래 움직임 복구
//	FTimerHandle TimerHandle;
//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Player]()
//	{
//		if (IsValid(Player))
//		{
//			Player->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);	
//		}
//	}, 0.5f, false);
//}

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

//void ABoss::Attack()
//{
//	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);
//	
//	if (!AnimInstance) return;
//
//	// If any montage is playing, prevent duplicate playback
//	if (AnimInstance->IsAnyMontagePlaying()) return;
//
//	if (IsValid(AttackAnimations[AttackType]))
//	{
//		ChaseAccumulatedTime = 0.f;
//		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABoss::OnAttackMontageEnded);
//		AnimInstance->OnMontageEnded.AddDynamic(this, &ABoss::OnAttackMontageEnded);
//		M_PlayMontage(AttackAnimations[AttackType]);
//	}
//
//	bIsAttacking = true;
//}

void ABoss::OnAttackEnded()
{
	Super::OnAttackEnded();
}

void ABoss::SetMoveSpeed(const float& SpeedMultiplier)
{
	//GetCharacterMovement()->MaxSwimSpeed = StatComponent->MoveSpeed * SpeedMultiplier;
	AquaticMovementComponent->MaxSpeed = StatComponent->MoveSpeed * SpeedMultiplier;
}


//void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
//{
//	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
//}

void ABoss::M_OnDeath_Implementation()
{
	//GetCharacterMovement()->StopMovementImmediately();

	if (IsValid(AnimInstance))
	{
		// 모든 애니메이션 출력 정지
		AnimInstance->StopAllMontages(0.5f);
	}
	
	
	// 사망 상태로 전이
	//SetBossState(EBossState::Death);
	SetMonsterState(EMonsterState::Death);
	
	// 피직스 에셋 물리엔진 적용
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABoss::ApplyPhysicsSimulation, 0.5f, false);
}

//void ABoss::DeathToRaderOff()
//{
//	URadarReturnComponent* RaderComponent = Cast<URadarReturnComponent>(GetComponentByClass(URadarReturnComponent::StaticClass()));
//	if (RaderComponent)
//	{
//		RaderComponent->SetIgnore(true);
//	}
//}

//void ABoss::ApplyPhysicsSimulation()
//{
//	switch (BossPhysicsType)
//	{
//		// 물리엔진 비활성화
//		case EBossPhysicsType::None:
//			break;
//
//		// Simulate Physic 적용
//		case EBossPhysicsType::SimulatePhysics:
//			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//			GetMesh()->SetEnableGravity(true);
//			GetMesh()->SetSimulatePhysics(true);	
//			break;
//	}
//}

FVector ABoss::GetNextPatrolPoint()
{
	return GetActorLocation();
}
#pragma endregion