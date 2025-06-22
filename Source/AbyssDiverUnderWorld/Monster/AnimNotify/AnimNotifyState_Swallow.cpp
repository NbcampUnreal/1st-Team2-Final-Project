// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotifyState_Swallow.h"
#include "Monster/HorrorCreature/HorrorCreature.h"
#include "AbyssDiverUnderWorld.h"

void UAnimNotifyState_Swallow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!MeshComp) return;

	AHorrorCreature* Monster = Cast<AHorrorCreature>(MeshComp->GetOwner());
	if (!Monster || !Monster->HasAuthority()) return;

	if (Monster->GetSwallowedPlayer() != nullptr) return; 

	UPrimitiveComponent* HitComp = Monster->GetAttackHitComponent();
	if (!HitComp) return;

	HitComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitComp->SetGenerateOverlapEvents(true);

	LOG(TEXT("[Swallow] Attack Collision Enabled"));
}

void UAnimNotifyState_Swallow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AHorrorCreature* Monster = Cast<AHorrorCreature>(MeshComp->GetOwner());
	if (!Monster || !Monster->HasAuthority()) return;

	UPrimitiveComponent* HitComp = Monster->GetAttackHitComponent();
	if (!HitComp) return;

	HitComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitComp->SetGenerateOverlapEvents(false);

	LOG(TEXT("[Swallow] Attack Collision Disabled"));
}
