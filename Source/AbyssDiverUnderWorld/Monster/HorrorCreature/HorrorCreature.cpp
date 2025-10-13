// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreature.h"
#include "Container/BlackboardKeys.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Monster/HorrorCreature/HorrorCreatureAIController.h"
#include "Kismet/GameplayStatics.h"


AHorrorCreature::AHorrorCreature()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize Variable
	ChaseTriggerTime = 2.0f;
	SwallowedPlayer = nullptr;
	FleeTime = 4.5f;
	SwallowDamage = 900.0f;

	HorrorCreatureHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HorrorCreatureHitSphere"));
	HorrorCreatureHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	HorrorCreatureHitSphere->InitSphereRadius(70.0f);
	HorrorCreatureHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HorrorCreatureHitSphere->SetHiddenInGame(true);
}

void AHorrorCreature::BeginPlay()
{
	Super::BeginPlay();
	
	if (AIController)
	{
		CachedPerceptionComponent = AIController->FindComponentByClass<UAIPerceptionComponent>();
	}
	HorrorCreatureHitSphere->OnComponentBeginOverlap.AddDynamic(this, &AHorrorCreature::OnSwallowTriggerOverlap);
}

void AHorrorCreature::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	// 먹는 순간만 활성화 되도록
	if (bSwallowingInProgress && SwallowedPlayer && IsValid(SwallowedPlayer))
	{
		// 먹을 때 플레이어 위치 입으로 빨려가도록 해주는 함수
		UpdateVictimLocation(DeltaTime);
	}

	// 도망가는 중에만 활성화 되도록
	if (MonsterState == EMonsterState::Flee && IsValid(BlackboardComponent))
	{
		// DesireLocation을 FleeLocation으로 업데이트
		SetDesireTargetLocation(BlackboardComponent->GetValueAsVector(BlackboardKeys::HorrorCreature::FleeLocationKey));
	}
}

void AHorrorCreature::UpdateVictimLocation(float DeltaTime)
{
	// 보간 SwallowSpeed 초기값 1.5 (삼키는 속도)
	SwallowLerpAlpha += DeltaTime * SwallowSpeed;

	// 오버슈트 방지 (몬스터 입을 넘는 위치 방지)
	const float Alpha = FMath::Clamp(SwallowLerpAlpha, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(VictimLocation, CreatureMouthLocation, Alpha);

	SwallowedPlayer->SetActorLocation(NewLoc);

	// 몬스터의 입 위치에 도달했다면
	if (Alpha >= 1.f)
	{
		// Attach
		SwallowedPlayer->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("MouthSocket"));
		SwallowedPlayer->SetActorRelativeLocation(FVector::ZeroVector);

		// 틱 비활성화
		bSwallowingInProgress = false;
	}
}

void AHorrorCreature::OnSwallowTriggerOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bCanSwallow || !HasAuthority() || SwallowedPlayer) return;
	if (OtherActor == this) return;
	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(OtherActor);
	if (!PlayerCharacter || PlayerCharacter->GetCharacterState() != ECharacterState::Normal) return;

	AUnderwaterCharacter* Victim = Cast<AUnderwaterCharacter>(OtherActor);
	if (Victim)
	{
		SwallowPlayer(Victim);
		UE_LOG(LogTemp, Warning, TEXT("SwallowPlayer is Called"))
	}
}

// 플레이어를 삼켰을 때 실행되는 함수
void AHorrorCreature::SwallowPlayer(AUnderwaterCharacter* Victim)
{
	if (!HasAuthority() || !Victim || SwallowedPlayer) return;
	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(Victim);
	if (!PlayerCharacter || PlayerCharacter->GetCharacterState() != ECharacterState::Normal) return;

	SwallowedPlayer = Victim;
	bCanSwallow = false;
	bSwallowingInProgress = true;

	// 데미지 처리
	DamageToVictim(Victim, SwallowDamage);

	// 플레이어의 시야 어둡게
	Victim->StartCaptureState();

	// 플레이어 위치, 크리처 입 위치 설정
	VictimLocation = Victim->GetActorLocation();
	CreatureMouthLocation = GetMesh()->GetSocketLocation("MouthSocket");
	SwallowLerpAlpha = 0.f;

	// Flee (도망가는) 상태로 변경
	ApplyMonsterStateChange(EMonsterState::Flee);

	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsBool(BlackboardKeys::HorrorCreature::bIsPlayerSwallowKey, true);
	}
}

void AHorrorCreature::EjectPlayer(AUnderwaterCharacter* Victim)
{
	if (!Victim) return;

	// 뱉자마자 플레이어를 인식하지 못하도록 일시적으로 Perception을 끔 (초기값 : 2초)
	TemporarilyDisalbeSightPerception(DisableSightTime);
	
	// 뱉은 플레이어 정상화 함수
	EjectedVictimNormalize(Victim);

	// 어그로 초기화
	ForceRemoveDetectedPlayers();

	SwallowedPlayer = nullptr;
	bCanSwallow = true;

	BlackboardComponent->SetValueAsBool(BlackboardKeys::HorrorCreature::bIsPlayerSwallowKey, false);
	
	FTimerHandle SetPatrolTimeHandle;
	GetWorld()->GetTimerManager().SetTimer(
		SetPatrolTimeHandle,
		this,
		&AHorrorCreature::SetPatrolStateAfterEject,
		FleeTime,
		false
	);
}

void AHorrorCreature::EjectedVictimNormalize(AUnderwaterCharacter* Victim)
{
	// 플레이어 뱉고, 어두워진 화면 제거
	Victim->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Victim->SetActorRotation(FRotator(0.f, Victim->GetActorRotation().Yaw, 0.f));
	Victim->StopCaptureState();

	// 플레이어를 입으로부터 앞으로 Lanch
	FVector LaunchDirection = GetActorForwardVector();
	FVector LaunchVelocity = LaunchDirection * LanchStrength;
	Victim->LaunchCharacter(LaunchVelocity, false, false);

	if (EjectMontage)
	{
		M_PlayMontage(EjectMontage);
	}

	// 플레이어 수영모드 On
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_SetSwimMode,
		[Victim]()
		{
			if (IsValid(Victim) && Victim->GetCharacterMovement())
			{
				Victim->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
			}
		},
		0.5f,
		false
	);
}

void AHorrorCreature::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	if (SwallowedPlayer) return;

	Super::NotifyLightExposure(DeltaTime, TotalExposedTime, PlayerLocation, PlayerActor);
}

void AHorrorCreature::OnDeath()
{
	Super::OnDeath();

	EjectPlayer(SwallowedPlayer);
}

// 일시적으로 Sight Perception을 끄는 함수
void AHorrorCreature::TemporarilyDisalbeSightPerception(float Duration)
{
	if (!CachedPerceptionComponent) return;

	CachedPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);

	// Duration (초기값 : 1.5초) 만큼 있다가 다시 Perception On
	FTimerHandle EnableSightHandle;
	GetWorld()->GetTimerManager().SetTimer(
		EnableSightHandle,
		this,
		&AHorrorCreature::SightPerceptionOn,
		Duration,
		false
	);
}

void AHorrorCreature::SightPerceptionOn()
{
	if (CachedPerceptionComponent)
	{
		CachedPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	}
}

void AHorrorCreature::SetPatrolStateAfterEject()
{
	ApplyMonsterStateChange(EMonsterState::Patrol);
}

void AHorrorCreature::DamageToVictim(AUnderwaterCharacter* Victim, float Damage)
{
	if (AIController && IsValid(Victim) && IsValid(this))
	{
		UGameplayStatics::ApplyDamage(
			Victim,
			Damage,
			AIController,
			this,
			UDamageType::StaticClass()
		);
	}
}
	
// void AHorrorCreature::InitializeAggroVariable()
// {
// 	if (!AIController || !IsValid(this) || !BlackboardComponent) return;
// 
// 	bIsChasing = false;
// 	TargetPlayer = nullptr;
// 	BlackboardComponent->ClearValue(BlackboardKeys::InvestigateLocationKey);
// }



