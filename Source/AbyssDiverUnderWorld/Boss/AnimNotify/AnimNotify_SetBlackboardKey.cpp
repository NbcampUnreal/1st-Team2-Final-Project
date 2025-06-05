#include "Boss/AnimNotify/AnimNotify_SetBlackboardKey.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

void UAnimNotify_SetBlackboardKey::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner());
	if (!IsValid(Boss)) return;
	
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(Boss->GetController());
	if (!IsValid(AIController)) return;

	for (auto KeyValue : BlackboardKeyValues)
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(FName(*KeyValue.Key), KeyValue.Value);
	}
}
