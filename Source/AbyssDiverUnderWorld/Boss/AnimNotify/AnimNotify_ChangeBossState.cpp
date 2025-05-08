#include "Boss/AnimNotify/AnimNotify_ChangeBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"

const FName UAnimNotify_ChangeBossState::BossStateKey = "BossState";

void UAnimNotify_ChangeBossState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(Character)) return;

	AAIController* AIController = Cast<AAIController>(Character->GetController());
	if (!IsValid(AIController)) return;

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	if (!IsValid(Blackboard)) return;

	Blackboard->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));
}
