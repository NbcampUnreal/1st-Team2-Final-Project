// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreature.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UnderwaterCharacter.h"

AHorrorCreature::AHorrorCreature()
{
	// Initialize Variable
	ChaseTriggerTime = 2.0f;
	ChaseSpeed = 650.0f;
	PatrolSpeed = 150.0f;
	InvestigateSpeed = 250.0f;
	SwallowedPlayer = nullptr;

	HorrorCreatureHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HorrorCreatureHitSphere"));
	HorrorCreatureHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	HorrorCreatureHitSphere->InitSphereRadius(20.0f);
	HorrorCreatureHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HorrorCreatureHitSphere->SetHiddenInGame(true);
}

void AHorrorCreature::BeginPlay()
{
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

	AUnderwaterCharacter* Victim = Cast<AUnderwaterCharacter>(OtherActor);
	if (Victim)
	{
		SwallowPlayer(Victim);
	}
}

void AHorrorCreature::SwallowPlayer(AUnderwaterCharacter* Victim)
{
	if (!HasAuthority() || !Victim || SwallowedPlayer) return;

	SwallowedPlayer = Victim;

	Victim->SetActorEnableCollision(false);
	Victim->GetCharacterMovement()->DisableMovement();
	Victim->SetActorHiddenInGame(true);
	// @TODO : Add SwallowedEffect on PlayerLogic
	// Victim->Client_PlaySwallowedEffect(); 

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &AHorrorCreature::EjectPlayer, Victim), 5.0f, false);
}

void AHorrorCreature::EjectPlayer(AUnderwaterCharacter* Victim)
{
	if (!Victim) return;

	// Re-enable components
	Victim->SetActorEnableCollision(true);
	Victim->SetActorHiddenInGame(false);
	Victim->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
	// @TODO : Add EjectEffect on PlayerLogic
	// Victim->Client_PlayEjectEffect();

	// Launch the player upward and forward from the mouth
	FVector LaunchDirection = GetActorForwardVector() + FVector(0, 0, 1.2f);
	LaunchDirection.Normalize();
	FVector LaunchVelocity = LaunchDirection * 150.f; // adjust strength as needed
	Victim->LaunchCharacter(LaunchVelocity, true, true);

	SwallowedPlayer = nullptr;
}

