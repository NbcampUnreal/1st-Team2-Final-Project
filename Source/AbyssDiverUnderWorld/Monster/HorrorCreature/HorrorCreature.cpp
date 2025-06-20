// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreature.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/HorrorCreature/HorrorCreatureAIController.h"
#include "Kismet/GameplayStatics.h"


AHorrorCreature::AHorrorCreature()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize Variable
	ChaseTriggerTime = 2.0f;
	ChaseSpeed = 1400.0f;
	PatrolSpeed = 200.0f;
	InvestigateSpeed = 500.0f;
	FleeSpeed = 2000.0f;
	SwallowedPlayer = nullptr;
	bCanSwallow = true;
	LanchStrength = 150.0f;
	DisableSightTime = 1.5f;
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

	if (bSwallowingInProgress && SwallowedPlayer && IsValid(SwallowedPlayer))
	{
		SwallowLerpAlpha += DeltaTime * 1.5f;
		const float Alpha = FMath::Clamp(SwallowLerpAlpha, 0.f, 1.f);
		const FVector NewLoc = FMath::Lerp(SwallowStartLocation, SwallowTargetLocation, Alpha);

		SwallowedPlayer->SetActorLocation(NewLoc);

		if (Alpha >= 1.f)
		{
			// Attach
			SwallowedPlayer->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("MouthSocket"));
			SwallowedPlayer->SetActorRelativeLocation(FVector::ZeroVector);

			// 상태 전이
			SetMonsterState(EMonsterState::Flee);
			BlackboardComponent->ClearValue(TargetActorKey);

			// 연출 종료
			bSwallowingInProgress = false;
		}
	}
}


void AHorrorCreature::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
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

	AUnderwaterCharacter* Victim = Cast<AUnderwaterCharacter>(OtherActor);
	if (Victim)
	{
		SwallowPlayer(Victim);
		UE_LOG(LogTemp, Warning, TEXT("SwallowPlayer is Called"))
	}
}

void AHorrorCreature::SwallowPlayer(AUnderwaterCharacter* Victim)
{
	if (!HasAuthority() || !Victim || SwallowedPlayer) return;

	SwallowedPlayer = Victim;
	bCanSwallow = false;

	DamageToVictim(Victim, SwallowDamage);

	Victim->StartCaptureState();

	// Set the Victim movement start position
	SwallowStartLocation = Victim->GetActorLocation();
	SwallowTargetLocation = GetMesh()->GetSocketLocation("MouthSocket");
	SwallowLerpAlpha = 0.f;
	bSwallowingInProgress = true;

	SetMonsterState(EMonsterState::Flee);
	BlackboardComponent->ClearValue(TargetActorKey);
}

void AHorrorCreature::EjectPlayer(AUnderwaterCharacter* Victim)
{
	if (!Victim) return;

	// Perception keeps setting the TargetActor, so it should be turned off.
	TemporarilyDisalbeSightPerception(DisableSightTime);

	Victim->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Victim->SetActorRotation(FRotator(0.f, Victim->GetActorRotation().Yaw, 0.f));
	Victim->StopCaptureState();
	
	// Launch the player upward and forward from the mouth
	FVector LaunchDirection = GetActorForwardVector();
	FVector LaunchVelocity = LaunchDirection * LanchStrength; // adjust strength as needed
	Victim->LaunchCharacter(LaunchVelocity, false, false);

	if (EjectMontage)
	{
		M_PlayMontage(EjectMontage);
	}

	// SwimMode On
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

	SwallowedPlayer = nullptr;
	bCanSwallow = true;
	InitializeAggroVariable();

	FTimerHandle SetPatrolTimeHandle;
	GetWorld()->GetTimerManager().SetTimer(
		SetPatrolTimeHandle,
		this,
		&AHorrorCreature::SetPatrolStateAfterEject,
		FleeTime,
		false
	);
}

void AHorrorCreature::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	if (SwallowedPlayer) return;

	Super::NotifyLightExposure(DeltaTime, TotalExposedTime, PlayerLocation, PlayerActor);
}

void AHorrorCreature::TemporarilyDisalbeSightPerception(float Duration)
{
	if (!CachedPerceptionComponent) return;

	CachedPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);

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
	SetMonsterState(EMonsterState::Patrol);
	BlackboardComponent->ClearValue(TargetActorKey);
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
	
void AHorrorCreature::InitializeAggroVariable()
{
	if (!AIController || !IsValid(this) || !BlackboardComponent) return;
	
	AIController->SetbIsLosingTarget(false);
	bIsChasing = false;
	TargetActor = nullptr;
	BlackboardComponent->ClearValue(InvestigateLocationKey);
}


