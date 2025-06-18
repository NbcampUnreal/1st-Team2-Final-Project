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

	SourceCharacter = nullptr;
	TargetCharacter = nullptr;
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

void ACableBindingActor::ConnectActors(AUnderwaterCharacter* NewSourceActor, AUnderwaterCharacter* NewTargetActor)
{
	if (!NewSourceActor || !NewTargetActor)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("ACableBindingActor::ConnectActors - Invalid actors provided."));
		return;
	}

	// Source : Cable의 시작점이 되는 Actor
	// Target : Cable의 끝점이 되는 Actor
	SourceCharacter = NewSourceActor;
	TargetCharacter = NewTargetActor;

	// Source Component 를 시작점으로 연결하고 Target Component를 끝점으로 연결한다.
	
	UPrimitiveComponent* SourceComponent = Cast<UPrimitiveComponent>(NewSourceActor->GetMesh());
	UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(NewTargetActor->GetMesh());
	
	CableComponent->SetAttachEndToComponent(SourceComponent, NAME_None);
	CableComponent->EndLocation = FVector::ZeroVector;

	if (SourceComponent && TargetComponent)
	{
		PhysicsConstraint->SetWorldLocation(SourceCharacter->GetActorLocation());
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
		UE_LOG(LogAbyssDiverCharacter, Warning,
		       TEXT("ACableBindingActor::ConnectActors - Actors do not have valid root components."));
	}
}

void ACableBindingActor::DisconnectActors()
{
	PhysicsConstraint->BreakConstraint();

	CableComponent->SetAttachEndTo(nullptr, NAME_None);

	SourceCharacter = nullptr;
	TargetCharacter = nullptr;
}

void ACableBindingActor::UpdateCable()
{
	if (!SourceCharacter || !TargetCharacter)
	{
		return;
	}

	// Root Component: CableComponent
	SetActorLocation(SourceCharacter->GetActorLocation());

	UPrimitiveComponent* SourceComponent = SourceCharacter->GetMesh();
	FVector TargetLocation = TargetCharacter->GetMesh()->GetBoneLocation("pelvis");
	// End Location은 Attach 된 Component의 상대 위치로 설정. 따라서 회전을 반영해야 한다.
	CableComponent->EndLocation = SourceComponent->GetComponentTransform().InverseTransformPosition(TargetLocation);
}

bool ACableBindingActor::IsConnected() const
{
	return SourceCharacter != nullptr && TargetCharacter != nullptr;
}
