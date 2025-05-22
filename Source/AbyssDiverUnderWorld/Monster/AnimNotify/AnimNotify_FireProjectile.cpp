// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotify_FireProjectile.h"
#include "Monster/GobleFish/GobleFish.h"

void UAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	AGobleFish* GobleFish = Cast<AGobleFish>(Owner);
	if (GobleFish && Owner->HasAuthority())
	{
		GobleFish->FireProjectile();
	}
}
