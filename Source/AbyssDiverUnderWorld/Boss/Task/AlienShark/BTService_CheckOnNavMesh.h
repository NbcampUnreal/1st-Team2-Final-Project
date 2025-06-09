#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckOnNavMesh.generated.h"

struct FBTCheckOnNavMeshMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<class AEnhancedBossAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<class ABoss> Boss;

	/** NavMesh 체크가 끝나는 시간 */
	float NavMeshCheckTimer = 0.0f;;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckOnNavMesh : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_CheckOnNavMesh();

private:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTCheckOnNavMeshMemory); }

	static const FName NavMeshCheckKey;
};
