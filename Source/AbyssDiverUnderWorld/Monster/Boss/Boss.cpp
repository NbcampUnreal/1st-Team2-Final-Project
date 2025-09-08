#include "Monster/Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "EnhancedBossAIController.h"
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
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "Monster/EPerceptionType.h"
#include "Framework/ADPlayerController.h"
#include "Container/BlackboardKeys.h"
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
		EnhancedAIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::TargetLocationKey, GetActorLocation());
	}

	GetCharacterMovement()->MaxSwimSpeed = StatComponent->GetMoveSpeed();
	OriginDeceleration = GetCharacterMovement()->BrakingDecelerationSwimming;
	CurrentMoveSpeed = StatComponent->MoveSpeed;

	SphereOverlapRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;

	Params.AddIgnoredActor(this);
}

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

#pragma endregion

#pragma region 보스 유틸 함수

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

void ABoss::SetMoveSpeed(const float& SpeedMultiplier)
{
	AquaticMovementComponent->MaxSpeed = StatComponent->MoveSpeed * SpeedMultiplier;
}

void ABoss::M_OnDeath_Implementation()
{
	if (IsValid(AnimInstance))
	{
		// 모든 애니메이션 출력 정지
		AnimInstance->StopAllMontages(0.5f);
	}

	SetMonsterState(EMonsterState::Death);
	
	// 피직스 에셋 물리엔진 적용
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABoss::ApplyPhysicsSimulation, 0.5f, false);
}

#pragma endregion