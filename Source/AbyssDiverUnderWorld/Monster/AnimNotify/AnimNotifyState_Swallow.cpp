// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotifyState_Swallow.h"
#include "Monster/HorrorCreature/HorrorCreature.h"
#include "AbyssDiverUnderWorld.h"

void UAnimNotifyState_Swallow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AHorrorCreature* Monster = Cast<AHorrorCreature>(MeshComp->GetOwner()))
	{
		if (Monster->GetAttackHitComponent())
		{
			Monster->GetAttackHitComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Monster->GetAttackHitComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
			Monster->GetAttackHitComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			LOG(TEXT("ShallowAttack Begin"));
		}
	}
}

void UAnimNotifyState_Swallow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AHorrorCreature* Monster = Cast<AHorrorCreature>(MeshComp->GetOwner()))
	{
		if (Monster->GetAttackHitComponent())
		{
			Monster->GetAttackHitComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			LOG(TEXT("ShallowAttack End"));
		}
	}
}
