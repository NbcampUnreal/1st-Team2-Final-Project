#include "Monster/Limadon/Limadon.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/Components/TickControlComponent.h"

#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Components/CapsuleComponent.h"

ALimadon::ALimadon()
{
	bIsInvestigate = true;
	HideTime = 30.0f;

	BiteAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Attack Collision"));
	BiteAttackCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteAttackCollision->SetCapsuleHalfHeight(80.0f);
	BiteAttackCollision->SetCapsuleRadius(80.0f);
	BiteAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteAttackCollision->ComponentTags.Add(TEXT("Bite Attack Collision"));

	LeftSphereMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Sphere Mesh"));
	LeftSphereMesh->SetupAttachment(GetMesh());
	// 가장 부하가 적음
	LeftSphereMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	RightSphereMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Sphere Mesh"));
	RightSphereMesh->SetupAttachment(GetMesh());
	// 가장 부하가 적음
	RightSphereMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	StopCaptureHealthCriteria = 1000.0f;

	bReplicates = true;

	bIsBiteAttackSuccess = false;
}

void ALimadon::BeginPlay()
{
	Super::BeginPlay();

	SetMonsterState(EMonsterState::Investigate);
	BiteAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ALimadon::OnBiteCollisionOverlapBegin);

	TickControlComponent->RegisterComponent(LeftSphereMesh);
	TickControlComponent->RegisterComponent(RightSphereMesh);

	if (AquaticMovementComponent)
	{
		TickControlComponent->UnregisterComponent(AquaticMovementComponent);
	}
}

float ALimadon::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float Damage = AUnitBase::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

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

			// Limadon은 피해를 입어도 어그로 끌리지 않음
		}
	}

	return Damage;
}

void ALimadon::BiteVariableInitialize()
{
	// HideTime 동안 공격 비활성화
	GetWorldTimerManager().SetTimer(InvestigateTimerHandle, this, &ALimadon::SetInvestigateMode, HideTime, false);

	// Bite 상태 변수 비활성화
	SetIsBiteAttackFalse();

	// 타겟 변수 초기화
	InitTarget();

	// 체력 초기화
	StatComponent->CurrentHealth = StatComponent->MaxHealth;
}

void ALimadon::Attack()
{
	Super::Attack();

	bIsInvestigate = false;
}

void ALimadon::OnDeath()
{
	Spit();
	
	Super::OnDeath();
}

void ALimadon::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	// Limadon은 빛에 반응 하지 않음
}

void ALimadon::ReceiveKnockback(const FVector& Force)
{
	// 넉백 당하지 않음.
}

void ALimadon::Spit()
{
	if (!IsValid(GetTarget()))
	{
		LOGV(Error, TEXT("Not Valid Target"));
		return;
	}

	// Emissive Color 감소
	SetEmissiveDown();

	// 플레이어 놓아줌
	GetTarget()->StopCaptureState();

	// 캐릭터 넉백
	LaunchPlayer(GetTarget(), LaunchPower);

	// 변수 초기화
	BiteVariableInitialize();

	ForceRemoveDetectedPlayers();
}

void ALimadon::OnBiteCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
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
	/*SetTarget(Player);*/
	AddDetection(Player);
	Player->StartCaptureState();
}
