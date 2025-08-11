#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
//#include "BTService_HasObstacleAhead.generated.h"

//struct FBTHasObstacleAheadMemory
//{
//	/**< 빙의한 AIController에 대한 참조 */
//	TWeakObjectPtr<class AEnhancedBossAIController> AIController;
//	
//	/** AIController의 주체에 대한 참조 */
//	TWeakObjectPtr<class ABoss> Boss;
//};
//
//UCLASS()
//class ABYSSDIVERUNDERWORLD_API UBTService_HasObstacleAhead : public UBTService_BlackboardBase
//{
//	GENERATED_BODY()
//
//public:
//	UBTService_HasObstacleAhead();
//
//private:
//	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
//	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
//	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
//	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTHasObstacleAheadMemory); }
//
//	static const FName bHasObstacleAheadKey;
//};
