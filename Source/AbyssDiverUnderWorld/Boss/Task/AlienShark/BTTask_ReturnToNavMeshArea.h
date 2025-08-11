#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
//#include "BTTask_ReturnToNavMeshArea.generated.h"

//struct FBTReturnToNavMeshAreaMemory
//{
//	/** 빙의한 AIController에 대한 참조 */
//	TWeakObjectPtr<class AEnhancedBossAIController> AIController;
//
//	/** AIController의 주체에 대한 참조 */
//	TWeakObjectPtr<class ABoss> Boss;
//	
//};
//
//UCLASS()
//class ABYSSDIVERUNDERWORLD_API UBTTask_ReturnToNavMeshArea : public UBTTask_BlackboardBase
//{
//	GENERATED_BODY()
//	
//public:
//	UBTTask_ReturnToNavMeshArea();
//
//private:
//	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
//	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTReturnToNavMeshAreaMemory); }
//};
