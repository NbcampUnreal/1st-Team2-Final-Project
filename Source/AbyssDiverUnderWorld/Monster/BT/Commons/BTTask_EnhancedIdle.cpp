#include "Monster/BT/Commons/BTTask_EnhancedIdle.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Monster.h"
//#include "Monster/Boss/EnhancedBossAIController.h"
//#include "Monster/Boss/ENum/EBossState.h"
#include "Monster/Components/AquaticMovementComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

UBTTask_EnhancedIdle::UBTTask_EnhancedIdle()
{
	NodeName = "Enhanced Idle";
	bNotifyTick = true;
	bCreateNodeInstance = false;
	
	IdleFinishMaxInterval = 2.0f;
	IdleFinishMinInterval = 0.5f;
}

EBTNodeResult::Type UBTTask_EnhancedIdle::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTIdleTaskMemory* TaskMemory = (FBTIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AMonsterAIController>(Comp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->Monster.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Monster->SetMonsterState(EMonsterState::Idle);
	TaskMemory->AccumulatedTime = 0.f;
	TaskMemory->IdleFinishTime = FMath::RandRange(IdleFinishMinInterval, IdleFinishMaxInterval);

	//TaskMemory->AIController->StopMovement();
	//TaskMemory->Monster->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	TaskMemory->Monster->AquaticMovementComponent->StopMovement();

	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedIdle::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTIdleTaskMemory* TaskMemory = (FBTIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	if (TaskMemory->AccumulatedTime > TaskMemory->IdleFinishTime)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}
