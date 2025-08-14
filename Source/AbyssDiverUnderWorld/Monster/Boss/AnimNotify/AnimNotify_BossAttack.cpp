#include "Monster/Boss/AnimNotify/AnimNotify_BossAttack.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/Boss/Boss.h"
#include "Components/CapsuleComponent.h"

UAnimNotify_BossAttack::UAnimNotify_BossAttack()
{
	AttackInterval = 0.2f;
	bIsMeshCollision = false;
}

void UAnimNotify_BossAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                    const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner());
	if (!IsValid(Boss)) return;

	// 메시 콜리전 가져오기
	if (bIsMeshCollision)
	{
		// AttackInterval 후 콜리전 비활성화
		Boss->GetWorldTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda([=]
		{
			if (IsValid(Boss))
			{
				Boss->OnAttackEnded();
			}
		}),AttackInterval, false);	
	}
	// 태그로 콜리전 가져오기
	else
	{
		UCapsuleComponent* AttackCollision = Boss->FindComponentByTag<UCapsuleComponent>(CollisionTag);
		if (!IsValid(AttackCollision)) return;
		
		// 오버랩 판정 시작
		AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
		// AttackInterval 후 콜리전 비활성화
		Boss->GetWorldTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda([=]
		{
			if (IsValid(AttackCollision))
			{
				AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			if (IsValid(Boss))
			{
				Boss->OnAttackEnded();
			}
		}),AttackInterval, false);	
	}
}
