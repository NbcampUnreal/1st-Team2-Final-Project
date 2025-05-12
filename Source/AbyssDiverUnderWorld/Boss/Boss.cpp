#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"
#include "EBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/GameplayStatics.h"

const FName ABoss::BossStateKey = "BossState";

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	
	TargetPlayer = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	AnimInstance = GetMesh()->GetAnimInstance();

	AIController = Cast<ABossAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
	}
}

void ABoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RotationToTarget();
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

void ABoss::OnAttackEnded()
{
	AttackedPlayers.Empty();
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

	// 플레이어를 밀치는 로직
	const FVector PushDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const float PushStrength = 1000.0f; // 밀치는 힘의 크기 -> 변수화 필요할 것 같은데 일단 고민
	const FVector PushForce = PushDirection * PushStrength;
	
	// 물리 시뮬레이션이 아닌 경우 LaunchCharacter 사용
	Player->LaunchCharacter(PushForce, false, false);

	// 0.5초 후 캐릭터의 원래 움직임 복구
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Player]()
	{
		Player->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
	}, 0.5f, false);
	
	LOG(TEXT("[Attack] %s"), *Player->GetName());
}

APawn* ABoss::GetTarget()
{
	if (IsValid(TargetPlayer)) 
	{
		return TargetPlayer;
	}
	
	return nullptr;
}

void ABoss::SetTarget(AUnderwaterCharacter* Target)
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