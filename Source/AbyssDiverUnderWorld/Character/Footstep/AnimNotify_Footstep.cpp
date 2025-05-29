// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_Footstep.h"

#include "FootstepComponent.h"
#include "Character/UnderwaterCharacter.h"

void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner)
	{
		return;
	}

	if (UFootstepComponent* FootstepComponent = Owner->FindComponentByClass<UFootstepComponent>())
	{
		FootstepComponent->PlayFootstepSound(FootType);
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("FootstepComponent not found on %s"), *Owner->GetName());
	}
}
