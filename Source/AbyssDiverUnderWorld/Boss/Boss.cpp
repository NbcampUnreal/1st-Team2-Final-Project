#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/StatComponent.h"
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
	GetCharacterMovement()->GravityScale = 0.1f;
	MoveStop();
	AnimInstance->StopAllMontages(0.5f);
	BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Death));
	if (IsValid(AIController))
	{
		AIController->UnPossess();
	}
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
	uint8 AttackType = FMath::RandRange(0, NormalAttackAnimations.Num() - 1);
	if (IsValid(NormalAttackAnimations[AttackType]))
	{
		M_PlayAnimation(NormalAttackAnimations[AttackType]);
	}
	else
	{
		LOG(TEXT(" [Boss] Attack Animation is not valid."));
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
		LOG(TEXT(" [Boss] Target Player is already set."));
		TargetPlayer = Target;
	}
}

void ABoss::SetLastDetectedLocation(const FVector& InLastDetectedLocation)
{
	LastDetectedLocation = InLastDetectedLocation;
}