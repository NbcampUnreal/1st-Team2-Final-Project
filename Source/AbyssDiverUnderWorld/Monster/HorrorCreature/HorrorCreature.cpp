// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreature.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "TimerManager.h"


AHorrorCreature::AHorrorCreature()
{
	// Initialize Variable
	ChaseTriggerTime = 2.0f;
	ChaseSpeed = 1400.0f;
	PatrolSpeed = 200.0f;
	InvestigateSpeed = 500.0f;
	FleeSpeed = 2000.0f;
	SwallowedPlayer = nullptr;
	bCanSwallow = true;
	LanchStrength = 150.0f;

	HorrorCreatureHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HorrorCreatureHitSphere"));
	HorrorCreatureHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	HorrorCreatureHitSphere->InitSphereRadius(20.0f);
	HorrorCreatureHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HorrorCreatureHitSphere->SetHiddenInGame(true);
}

void AHorrorCreature::BeginPlay()
{
	Super::BeginPlay();
	HorrorCreatureHitSphere->OnComponentBeginOverlap.AddDynamic(this, &AHorrorCreature::OnSwallowTriggerOverlap);
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

	// Attach to mouth or body socket
	Victim->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MouthSocket"));
	Victim->SetActorRelativeLocation(FVector::ZeroVector); // Safely reset location
	Victim->StartCaptureState();

	SetMonsterState(EMonsterState::Flee);
	RemoveDetection(Victim);
	BlackboardComponent->SetValueAsEnum(MonsterStateKey, static_cast<uint8>(EMonsterState::Flee));
}

void AHorrorCreature::EjectPlayer(AUnderwaterCharacter* Victim)
{
	if (!Victim) return;

	Victim->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Victim->SetActorHiddenInGame(false);
	Victim->StopCaptureState();
	
	// Launch the player upward and forward from the mouth
	FVector LaunchDirection = GetActorForwardVector();
	FVector LaunchVelocity = LaunchDirection * LanchStrength; // adjust strength as needed
	Victim->LaunchCharacter(LaunchVelocity, false, false);

	if (EjectMontage)
	{
		M_PlayMontage(EjectMontage);
	}

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

	SetMonsterState(EMonsterState::Patrol);
	BlackboardComponent->SetValueAsEnum(MonsterStateKey, static_cast<uint8>(EMonsterState::Patrol));

	SwallowedPlayer = nullptr;
	bCanSwallow = true;
}



