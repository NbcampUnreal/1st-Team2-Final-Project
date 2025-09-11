#include "Monster/AnimNotify/AnimNotify_ChangeMonsterState.h"

#include "Monster/Monster.h"

void UAnimNotify_ChangeMonsterState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AMonster* Monster = Cast<AMonster>(MeshComp->GetOwner());
	if (!IsValid(Monster)) return;

	Monster->SetMonsterState(MonsterState);
}
