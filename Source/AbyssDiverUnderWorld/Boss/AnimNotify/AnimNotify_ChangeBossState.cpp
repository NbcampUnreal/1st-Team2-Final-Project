#include "Boss/AnimNotify/AnimNotify_ChangeBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"

const FName UAnimNotify_ChangeBossState::BossStateKey = "BossState";

void UAnimNotify_ChangeBossState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
									const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner());
	if (!IsValid(Boss)) return;

	Boss->SetBossState(BossState);
}
