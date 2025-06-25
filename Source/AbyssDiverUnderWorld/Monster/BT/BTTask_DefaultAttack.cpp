// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_DefaultAttack.h"
#include "Monster/Monster.h"
#include "Monster/EMonsterState.h"
#include "AIController.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = "DefaultAttack";
	bCreateNodeInstance = true;
	bNotifyTick = false;
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_DefaultAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	if (!AIPawn) return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIPawn);
	if (!Monster) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	
	// In Server
	const bool bIsAttacking = Blackboard->GetValueAsBool(IsAttackingKey.SelectedKeyName);
	if (Monster->HasAuthority() && !bIsAttacking)
	{
		// Sync to Client
		Monster->PlayAttackMontage();
		Blackboard->SetValueAsBool(IsAttackingKey.SelectedKeyName, true);
		// Detect end of montage �� End BTTask
		if (UAnimInstance* AnimInst = AIPawn->GetMesh()->GetAnimInstance())
		{
			// Defence logic. Delegate duplication prevention.
			if (AnimInst->OnMontageEnded.IsAlreadyBound(this, &UBTTask_DefaultAttack::HandleAttackMontageEnded))
			{
				UE_LOG(LogTemp, Warning, TEXT("OnMontageEnded already bound! Removing and rebinding."));
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UBTTask_DefaultAttack::HandleAttackMontageEnded);
			}

			AnimInst->OnMontageEnded.AddDynamic(this, &UBTTask_DefaultAttack::HandleAttackMontageEnded);
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_DefaultAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Delegate remove
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn()))
		{
			if (UAnimInstance* AnimInst = AIPawn->GetMesh()->GetAnimInstance())
			{
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UBTTask_DefaultAttack::HandleAttackMontageEnded);
			}
		}
	}
}

void UBTTask_DefaultAttack::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CachedOwnerComp)
	{
		if (UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent())
		{
			// Reverting back to tracking now that the attack is over
			Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Chase));
			Blackboard->SetValueAsBool(IsAttackingKey.SelectedKeyName, false);
		}
		bInterrupted ? FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed) : FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}
