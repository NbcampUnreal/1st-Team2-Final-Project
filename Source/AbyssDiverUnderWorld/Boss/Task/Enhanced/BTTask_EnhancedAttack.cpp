#include "Boss/Task/Enhanced/BTTask_EnhancedAttack.h"

#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EPerceptionType.h"

UBTTask_EnhancedAttack::UBTTask_EnhancedAttack()
{
	NodeName = "Enhanced Attack";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
	bHasAttacked = false;
}

EBTNodeResult::Type UBTTask_EnhancedAttack::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	bHasAttacked = false;
	
	AnimInstance = Boss->GetAnimInstance();
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_EnhancedAttack::FinishPerception);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedAttack::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	if (bHasAttacked) return;
	
	if (Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		Boss->Attack();
		bHasAttacked = true;
	}
}

void UBTTask_EnhancedAttack::FinishPerception(UAnimMontage* Montage, bool bInterrupted)
{
	LOG(TEXT("Begin"));
	if (IsValid(AIController))
	{
		AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}
	
	// 바인딩 해제 (중복 방지)
	if (IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_EnhancedAttack::FinishPerception);
	}

}
