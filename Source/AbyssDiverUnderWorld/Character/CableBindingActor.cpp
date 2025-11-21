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

	if (IsConnected() && !IsConnectionValid())
	{
		DisconnectActors();
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
	// - CableComponent : Source Component에 Attach, End Location을 지정
	// - PhysicsConstraint : Source Character의 World Location을 추적

	// Source Component, Target Component는 Mesh Component에 연결
	UPrimitiveComponent* SourceComponent = Cast<UPrimitiveComponent>(NewSourceActor->GetMesh());
	UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(NewTargetActor->GetMesh());

	AttachToComponent(SourceComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("pelvis"));
	CableComponent->SetAttachEndToComponent(TargetComponent, FName("pelvis"));
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

bool ACableBindingActor::IsConnectionValid() const
{
	return IsValid(SourceCharacter) && IsValid(TargetCharacter);
}

void ACableBindingActor::ApplyWorldOffsetToCable(FVector Offset)
{
	if (CableComponent)
	{
		CableComponent->ApplyWorldOffset(Offset, false);
	}
}

bool ACableBindingActor::IsConnected() const
{
	return SourceCharacter != nullptr && TargetCharacter != nullptr;
}
