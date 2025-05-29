#include "Boss/Limadon/Limadon.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Enum/EBossState.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"

ALimadon::ALimadon()
{
	BossState = EBossState::Investigate;
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

	RightSphereMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Sphere Mesh"));
	RightSphereMesh->SetupAttachment(GetMesh());
	
	StopCaptureHealthCriteria = 1000.0f;

	bReplicates = true;
}

void ALimadon::BeginPlay()
{
	Super::BeginPlay();

	SetBossState(EBossState::Investigate);
	BiteAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnBiteCollisionOverlapBegin);
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

void ALimadon::Spit()
{
	if (!IsValid(GetTarget())) return;

	// Emissive Color 감소
	SetEmissiveDown();

	// 플레이어 놓아줌
	GetTarget()->StopCaptureState();

	// 캐릭터 넉백
	LaunchPlayer(GetTarget(), LaunchPower);

	// 변수 초기화
	BiteVariableInitialize();
}
