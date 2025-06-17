#include "Boss/Kraken/Kraken.h"
#include "AbyssDiverUnderWorld.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/EnhancedBossAIController.h"
#include "Boss/Enum/EBossState.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Gimmic/Volume/BattleFieldVolume.h"

AKraken::AKraken()
{
	BiteAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Attack Collision"));
	BiteAttackCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteAttackCollision->SetCapsuleHalfHeight(80.0f);
	BiteAttackCollision->SetCapsuleRadius(80.0f);
	BiteAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteAttackCollision->ComponentTags.Add(TEXT("Bite Attack Collision"));

	TakeDownAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TakeDown Attack Collision"));
	TakeDownAttackCollision->SetupAttachment(GetMesh(), TEXT("TakeDownSocket"));
	TakeDownAttackCollision->SetCapsuleHalfHeight(80.0f);
	TakeDownAttackCollision->SetCapsuleRadius(80.0f);
	TakeDownAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TakeDownAttackCollision->ComponentTags.Add(TEXT("TakeDown Attack Collision"));

	WieldAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Wield Attack Collision"));
	WieldAttackCollision->SetupAttachment(GetMesh(), TEXT("WieldSocket"));
	WieldAttackCollision->SetCapsuleHalfHeight(80.0f);
	WieldAttackCollision->SetCapsuleRadius(80.0f);
	WieldAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WieldAttackCollision->ComponentTags.Add(TEXT("Wield Attack Collision"));

	PickAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Pick Attack Collision"));
	PickAttackCollision->SetupAttachment(GetMesh(), TEXT("PickSocket"));
	PickAttackCollision->SetCapsuleHalfHeight(80.0f);
	PickAttackCollision->SetCapsuleRadius(80.0f);
	PickAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickAttackCollision->ComponentTags.Add(TEXT("Pick Attack Collision"));
}

void AKraken::BeginPlay()
{
	Super::BeginPlay();

	GetBattleFieldVolumeInWorld();

	if (IsValid(BattleFieldVolume))
	{
		BattleFieldVolume->OnBattleFieldBeginOverlapDelegate.AddUObject(this, &AKraken::OnBattleFieldBeginOverlap);
		BattleFieldVolume->OnBattleFieldEndOverlapDelegate.AddUObject(this, &AKraken::OnBattleFieldEndOverlap);
	}
	
	BiteAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	TakeDownAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	WieldAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	PickAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
}

void AKraken::OnDeath()
{
	if (BossState == EBossState::Death) return;
	SetEmissiveTransition();
	
	Super::OnDeath();
}

void AKraken::OnBattleFieldBeginOverlap()
{
	bCanBattle = true;
	EnhancedAIController->SetSightRadius(4000.f);
}

void AKraken::OnBattleFieldEndOverlap(const uint8& PlayerCount)
{
	if (PlayerCount == 0)
	{
		bCanBattle = false;
		
		AIController->GetBlackboardComponent()->SetValueAsBool("bIsChasing", false);
		AIController->GetBlackboardComponent()->SetValueAsBool("bCanAttack", false);
		EnhancedAIController->SetSightRadius(0.f);
	}
}

void AKraken::GetBattleFieldVolumeInWorld()
{
	for (ABattleFieldVolume* Volume : TActorRange<ABattleFieldVolume>(GetWorld()))
	{
		if (!IsValid(Volume)) continue;

		if (IsOverlappingActor(Volume))
		{
			LOG(TEXT("BattleFieldVolume Find Success !"))
			BattleFieldVolume = Volume;
			return;
		}
	}

	LOG(TEXT("-------------- Bug : Can't Find BattleFieldVolume !! --------------"));
}
