﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "FootstepComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SoundSubsystem.h"

UFootstepComponent::UFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bUseCharacterOnLanded = true;
	TraceHeight = 10.0f;
	TraceDistance = 100.0f;

	WalkFootstepSound = ESFX::WalkFootstep;
	LandFootstepSound = ESFX::LandFootstep;

	MinimumFootstepInterval = 0.1f;
	LastFootstepTime = -1.0f;
}

void UFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (bUseCharacterOnLanded)
	{
		OwnerCharacter->LandedDelegate.AddDynamic(this, &UFootstepComponent::OnLanded);
	}
}

void UFootstepComponent::PlayFootstepSound(const FVector& StartLocation)
{
	PlayFootstepSoundInternal(StartLocation);
}

void UFootstepComponent::PlayFootstepSound(const EFootSide FootSide)
{
	if (!IsValid(OwnerCharacter) || !OwnerCharacter->GetMesh())
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("OwnerCharacter or Mesh is not valid."));
		return;
	}

	FVector FootLocation = OwnerCharacter->GetActorLocation();
	FName FootBoneName = 
		FootSide == EFootSide::Left  ? LeftFootSocketName  : 
		FootSide == EFootSide::Right ? RightFootSocketName : 
		NAME_None;
	if (OwnerCharacter->GetMesh()->DoesSocketExist(FootBoneName))
	{
		FootLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FootBoneName);
	}

	PlayFootstepSoundInternal(FootLocation);
}

void UFootstepComponent::PlayLandSound()
{
	// 착지 소리는 항상 재생되어야 한다.
	// 착지 이후의 발소리는 최소 간격을 둔다.
	LastFootstepTime = GetWorld()->GetTimeSeconds();
	
	const EPhysicalSurface PhysicalSurface = GetSurfaceType(OwnerCharacter->GetActorLocation());
	
	if (ESFX LandSound = FindSound(PhysicalSurface, EFootstepType::Land);
		LandSound != ESFX::Max && GetSoundSubsystem())
	{
		GetSoundSubsystem()->PlayAt(LandSound, OwnerCharacter->GetActorLocation());
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Land sound not found for surface type %d"), static_cast<int32>(PhysicalSurface));
	}
}

void UFootstepComponent::PlayFootstepSoundInternal(const FVector& StartLocation)
{
	// 발소리 재생이 겹치지 않게 최소 간격을 둔다.
	if (GetWorld()->GetTimeSeconds() - LastFootstepTime < MinimumFootstepInterval)
	{
		return;
	}

	LastFootstepTime = GetWorld()->GetTimeSeconds();
	
	const EPhysicalSurface PhysicalSurface = GetSurfaceType(StartLocation);
	if (ESFX FootstepSound = FindSound(PhysicalSurface, EFootstepType::Walk);
		FootstepSound != ESFX::Max && GetSoundSubsystem())
	{
		GetSoundSubsystem()->PlayAt(FootstepSound, OwnerCharacter->GetActorLocation());
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Footstep sound not found for surface type %d"), static_cast<int32>(PhysicalSurface));
	}
}

EPhysicalSurface UFootstepComponent::GetSurfaceType(const FVector& Start) const
{
	if (!IsValid(OwnerCharacter))
	{
		return EPhysicalSurface::SurfaceType_Default;
	}
	
	FHitResult LandHitResult;
	const FVector StartLocation = Start + FVector::UpVector * TraceHeight;
	const FVector EndLocation = Start + FVector::DownVector * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());
	
	if (GetWorld()->LineTraceSingleByChannel(LandHitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, QueryParams))
	{
		return UGameplayStatics::GetSurfaceType(LandHitResult);
	}
	return EPhysicalSurface::SurfaceType_Default;
}

ESFX UFootstepComponent::FindSound(EPhysicalSurface SurfaceType, EFootstepType FootstepType) const
{
	return FootstepType == EFootstepType::Walk ? WalkFootstepSound : LandFootstepSound;
}

void UFootstepComponent::OnLanded(const FHitResult& Hit)
{
	PlayLandSound();
}

USoundSubsystem* UFootstepComponent::GetSoundSubsystem()
{
	if (!SoundSubsystem.IsValid())
	{
		if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
		{
			SoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>();
		}
		else
		{
			UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("SoundSubsystem is not valid and GameInstance is not found."));
		}
	}

	return SoundSubsystem.Get();
}
