// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_RotateToMovement.h"
#include "AIController.h"
#include "Monster/Monster.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_RotateToMovement::UBTService_RotateToMovement()
{
	NodeName = "Rotate To Movement Direction";
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	InterpSpeed = 5.0f;
}

void UBTService_RotateToMovement::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	AMonster* Monster = Cast<AMonster>(AIController->GetPawn());
	if (!Monster) return;

	UCharacterMovementComponent* MovementComp = Monster->GetCharacterMovement();
	if (!MovementComp) return;

	// No rotation when stationary
	const FVector MonsterVelocity = MovementComp->Velocity;
	if (MonsterVelocity.SizeSquared() < KINDA_SMALL_NUMBER) return;

	// Movement direction ¡æ Rotate
	FRotator TargetRotation = MonsterVelocity.GetSafeNormal().Rotation();
	TargetRotation.Roll = 0.0f;

	// Interpolation
	FRotator NewRotation = FMath::RInterpTo(Monster->GetActorRotation(), TargetRotation, DeltaSeconds, InterpSpeed);
	Monster->SetActorRotation(NewRotation);
}
