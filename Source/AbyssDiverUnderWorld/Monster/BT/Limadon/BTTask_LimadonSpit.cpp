#include "Monster/BT/Limadon/BTTask_LimadonSpit.h"

#include "Monster/Boss/Limadon/Limadon.h"
#include "Character/UnderwaterCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_LimadonSpit::UBTTask_LimadonSpit()
{
	NodeName = TEXT("Limadon Spit");
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_LimadonSpit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTLimadonSpitTaskMemory* TaskMemory = (FBTLimadonSpitTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;
	
	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Limadon->M_PlayMontage(TaskMemory->Limadon->SpitAnimation);
	
	return EBTNodeResult::Succeeded;
}