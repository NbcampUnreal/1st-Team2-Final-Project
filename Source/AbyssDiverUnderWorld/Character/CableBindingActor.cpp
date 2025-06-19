// Fill out your copyright notice in the Description page of Project Settings.
#include "CableBindingActor.h"
#include "CableComponent.h"

#include "UnderwaterCharacter.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


// Sets default values
ACableBindingActor::ACableBindingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CableLength = 500.0f;
	ConstraintStiffness = 1500.0f;
	ConstraintDamping = 200.0f;

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	SetRootComponent(CableComponent);
	CableComponent->CableWidth = 5.0f;
	CableComponent->NumSegments = 20;

	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(CableComponent);

	SourceActor = nullptr;
	TargetActor = nullptr;
}

void ACableBindingActor::BeginPlay()
{
	Super::BeginPlay();
}

void ACableBindingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsConnected())
	{
		UpdateCable();
	}
}

void ACableBindingActor::ConnectActors(AActor* NewSourceActor, AActor* NewTargetActor)
{
	if (!NewSourceActor || !NewTargetActor)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("ACableBindingActor::ConnectActors - Invalid actors provided."));
		return;
	}

	CableComponent->SetAttachEndToComponent(NewTargetActor->GetRootComponent(), NAME_None);
	CableComponent->EndLocation = FVector::ZeroVector;

	UPrimitiveComponent* SourceComponent = Cast<UPrimitiveComponent>(NewSourceActor->GetRootComponent());
	UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(NewTargetActor->GetRootComponent());

	if (SourceComponent && TargetComponent)
	{
		PhysicsConstraint->SetWorldLocation(SourceActor->GetActorLocation());
		PhysicsConstraint->SetConstrainedComponents(SourceComponent, NAME_None, TargetComponent, NAME_None);

		PhysicsConstraint->SetLinearXLimit(LCM_Limited, CableLength);
		PhysicsConstraint->SetLinearYLimit(LCM_Limited, CableLength);
		PhysicsConstraint->SetLinearZLimit(LCM_Limited, CableLength);

		PhysicsConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);
		PhysicsConstraint->SetAngularSwing2Limit(ACM_Free, 0.0f);
		PhysicsConstraint->SetAngularTwistLimit(ACM_Free, 0.0f);

		PhysicsConstraint->SetLinearPositionDrive(true, true, true);
		PhysicsConstraint->SetLinearVelocityDrive(true, true, true);
		PhysicsConstraint->SetLinearDriveParams(ConstraintStiffness, ConstraintDamping, 0.0f);
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("ACableBindingActor::ConnectActors - Actors do not have valid root components."));
	}
}

void ACableBindingActor::DisconnectActors()
{
	PhysicsConstraint->BreakConstraint();

	CableComponent->SetAttachEndTo(nullptr, NAME_None);

	SourceActor = nullptr;
	TargetActor = nullptr;
}

void ACableBindingActor::UpdateCable()
{
	if (!SourceActor || !TargetActor)
	{
		return;
	}

	SetActorLocation(SourceActor->GetActorLocation());
	CableComponent->EndLocation = TargetActor->GetActorLocation() - SourceActor->GetActorLocation();

	PhysicsConstraint->SetWorldLocation(SourceActor->GetActorLocation());
}

bool ACableBindingActor::IsConnected() const
{
	return SourceActor != nullptr && TargetActor != nullptr; 
}

