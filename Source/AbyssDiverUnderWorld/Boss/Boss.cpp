#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/StatComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"

const FName ABoss::BossStateKey = "BossState";

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BlackboardComponent = nullptr;
	AIController = nullptr;
	TargetPlayer = nullptr;
	LastDetectedLocation = FVector::ZeroVector;
	AttackRadius = 500.0f;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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
}

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	AActor* DamageCauser)
{
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
	MoveStop();
	AnimInstance->StopAllMontages(0.5f);

	// 사망 상태로 전이
	BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Death));

	// AIController 작동 중지
	AIController->UnPossess();
}

void ABoss::RotationToTarget()
{
	if (!IsValid(TargetPlayer)) return;

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (TargetPlayer->GetActorLocation() - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
	
	SetActorRotation(NewRotation);
}

void ABoss::Move()
{
	//M_PlayAnimation(MoveAnimation);
}

void ABoss::MoveStop()
{
	//M_PlayAnimation(IdleAnimation);
}

void ABoss::MoveToTarget()
{
	if (!IsValid(TargetPlayer)) return;

	const FVector TargetLocation = TargetPlayer->GetActorLocation();
	const FVector CurrentLocation = GetActorLocation();
	const float Distance = FVector::Distance(CurrentLocation, TargetLocation);

	// 타겟이 공격 반경 내에 있다면 Attack 상태로 전이
	if (Distance <= AttackRadius)
	{
		BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Attack));
	}
}

void ABoss::MoveToLastDetectedLocation()
{

}

void ABoss::Attack()
{
	const uint8 AttackType = FMath::RandRange(0, NormalAttackAnimations.Num() - 1);
	
	if (IsValid(NormalAttackAnimations[AttackType]))
	{
		M_PlayAnimation(NormalAttackAnimations[AttackType]);
	}
}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

APawn* ABoss::GetTarget()
{
	if (IsValid(TargetPlayer)) 
	{
		return TargetPlayer;
	}
	
	return nullptr;
}

void ABoss::SetTarget(APawn* Target)
{
	if (IsValid(Target))
	{
		TargetPlayer = Target;
	}
}

void ABoss::SetLastDetectedLocation(const FVector& InLastDetectedLocation)
{
	LastDetectedLocation = InLastDetectedLocation;
}