// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotify_MonsterDefaultAttack.h"
#include "Monster/Monster.h"
#include "Components/CapsuleComponent.h"

UAnimNotify_MonsterDefaultAttack::UAnimNotify_MonsterDefaultAttack()
{
	AttackInterval = 0.2f;
	bIsMeshCollision = false;
}

void UAnimNotify_MonsterDefaultAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AMonster* Monster = Cast<AMonster>(MeshComp->GetOwner());
	if (!IsValid(Monster)) return;

	// 메시 콜리전 가져오기
	if (bIsMeshCollision)
	{
		// AttackInterval 후 콜리전 비활성화
		Monster->GetWorldTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda([=]
			{
				if (IsValid(Monster))
				{
					Monster->OnAttackEnded();
				}
			}), AttackInterval, false);
	}
	// 태그로 콜리전 가져오기
	else
	{
		UCapsuleComponent* AttackCollision = Monster->FindComponentByTag<UCapsuleComponent>(CollisionTag);
		if (!IsValid(AttackCollision)) return;

		// 오버랩 판정 시작
		AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// AttackInterval 후 콜리전 비활성화
		Monster->GetWorldTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda([=]
			{
				if (IsValid(AttackCollision))
				{
					AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}

				if (IsValid(Monster))
				{
					Monster->OnAttackEnded();
				}
			}), AttackInterval, false);
	}
}
