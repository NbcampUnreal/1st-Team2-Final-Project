// Fill out your copyright notice in the Description page of Project Settings.

#include "RagdollReplicationComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

URagdollReplicationComponent::URagdollReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bIsRagdoll = false;
}


// Called when the game starts
void URagdollReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (!SkeletalMesh)
		{
			UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("URagdollReplicationComponent: SkeletalMeshComponent not found on %s"), *Owner->GetName());
		}
	}
}

void URagdollReplicationComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URagdollReplicationComponent, BoneTransforms);
}

void URagdollReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_Authority && bIsRagdoll && SkeletalMesh)
	{
		TimeSinceLastSnapshot += DeltaTime;
		if (TimeSinceLastSnapshot >= SnapshotInterval)
		{
			CaptureBoneTransforms();
			TimeSinceLastSnapshot = 0.0f;
		}
	}
}

void URagdollReplicationComponent::SetRagdollEnabled(bool bEnable)
{
	if (bIsRagdoll == bEnable || !SkeletalMesh)
	{
		return;
	}

	bIsRagdoll = bEnable;

	if (bIsRagdoll)
	{
		SkeletalMesh->SetSimulatePhysics(true);
		SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Block);
		// Interaction 채널에서 검출하기 위해서는 Visibility에서 Block으로 설정해야 한다.
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
		
		SkeletalMesh->SetEnableGravity(false);
		SkeletalMesh->SetLinearDamping(4.0f);
		SkeletalMesh->SetAngularDamping(4.0f);

		if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(GetOwner()))
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Character->GetCharacterMovement()->DisableMovement();
		}
	}
}

void URagdollReplicationComponent::CaptureBoneTransforms()
{
	if (!SkeletalMesh || CaptureBoneNames.IsEmpty())
	{
		return;
	}

	BoneTransforms.Empty();
	
	for (const FName& BoneName : CaptureBoneNames)
	{
		// 이유를 알 수 없으나 Root를 캡처하지 않으면 제대로 동작하지 않는다.
		// Root는 BodyInstance가 없어서 제대로 캡처가 되지 않는데도 일단은 동작한다.
		int32 BoneIndex = SkeletalMesh->GetBoneIndex(BoneName);
		if (BoneIndex != INDEX_NONE)
		{
			FBodyInstance* BodyInstance = SkeletalMesh->GetBodyInstance(BoneName);
			FTransform BodyTransform = BodyInstance ? BodyInstance->GetUnrealWorldTransform() : FTransform::Identity;
			FVector LinearVelocity = BodyInstance ? BodyInstance->GetUnrealWorldVelocity() : FVector::ZeroVector;
			FVector AngularVelocity = BodyInstance ? BodyInstance->GetUnrealWorldAngularVelocityInRadians() : FVector::ZeroVector;
			BoneTransforms.Emplace(BodyTransform.GetLocation(), 
			                   BodyTransform.GetRotation().Rotator(), 
			                   LinearVelocity, 
			                   AngularVelocity
			);
		}
	}
}

void URagdollReplicationComponent::OnRep_BoneTransforms()
{
	if (!SkeletalMesh || !bIsRagdoll)
	{
		return;
	}

	for (int32 i = 0; i < BoneTransforms.Num(); ++i)
	{
		FReplicatedBoneTransform BoneTransform = BoneTransforms[i];
		const FName& BoneName = CaptureBoneNames.IsValidIndex(i) ? CaptureBoneNames[i] : NAME_None;
		if (BoneName != NAME_None)
		{
			int32 BoneIndex = SkeletalMesh->GetBoneIndex(BoneName);
			if (BoneIndex != INDEX_NONE)
			{
				if (FBodyInstance* BodyInstance = SkeletalMesh->GetBodyInstance(BoneName))
				{
					BodyInstance->SetBodyTransform(BoneTransform.ToTransform(), ETeleportType::ResetPhysics);
					BodyInstance->SetLinearVelocity(BoneTransform.LinearVelocity, false);
					BodyInstance->SetAngularVelocityInRadians(BoneTransform.AngularVelocity, false);
				}
			}
		}
	}

	SkeletalMesh->WakeAllRigidBodies();
}