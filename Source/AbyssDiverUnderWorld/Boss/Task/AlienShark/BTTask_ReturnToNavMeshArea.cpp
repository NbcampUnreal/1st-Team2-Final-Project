#include "Boss/Task/AlienShark/BTTask_ReturnToNavMeshArea.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

//UBTTask_ReturnToNavMeshArea::UBTTask_ReturnToNavMeshArea()
//{
//	NodeName = TEXT("Return To NavMesh Area");
//	bCreateNodeInstance = false;
//	bNotifyTick = false;
//}
//
//EBTNodeResult::Type UBTTask_ReturnToNavMeshArea::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
//{
//	FBTReturnToNavMeshAreaMemory* TaskMemory = (FBTReturnToNavMeshAreaMemory*)NodeMemory;
//	if (!TaskMemory) return EBTNodeResult::Failed;
//
//	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
//	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
//
//	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return EBTNodeResult::Failed;
//
//	//TaskMemory->Boss->ReturnToNavMeshArea();
//
//	return EBTNodeResult::Succeeded;
//}

