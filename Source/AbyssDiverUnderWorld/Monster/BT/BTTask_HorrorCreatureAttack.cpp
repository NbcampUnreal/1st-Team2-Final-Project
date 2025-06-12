// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_HorrorCreatureAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

UBTTask_HorrorCreatureAttack::UBTTask_HorrorCreatureAttack()
{
	NodeName = "HorrorCreature Attack";
	bCreateNodeInstance = true;
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	CachedHorrorCreature = nullptr;
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_HorrorCreatureAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	if (!AIPawn) return EBTNodeResult::Failed;

	AHorrorCreature* Monster = Cast<AHorrorCreature>(AIPawn);
	CachedHorrorCreature = Monster;
	if (!Monster) return EBTNodeResult::Failed;

	if (!Monster->GetbCanSwallow()) return EBTNodeResult::Failed;

	UAnimInstance* AnimInst = AIPawn->GetMesh()->GetAnimInstance();
	for (UAnimMontage* AttackMontage : Monster->GetAttackAnimations())
	{
		if (AnimInst && AttackMontage && AnimInst->Montage_IsPlaying(AttackMontage))
		{
			// Already playing attack montage ¡æ Prevent duplicate execution
			return EBTNodeResult::Failed;
		}
	}
	// In Server
	// AIController->StopMovement();
	if (Monster->HasAuthority())
	{
		// Sync to Client
		Monster->PlayAttackMontage();

		// Detect end of montage ¡æ End BTTask
		if (AnimInst)
		{
			// Defence logic. Delegate duplication prevention.
			if (AnimInst->OnMontageEnded.IsAlreadyBound(this, &UBTTask_HorrorCreatureAttack::HandleAttackMontageEnded))
			{
				UE_LOG(LogTemp, Warning, TEXT("OnMontageEnded already bound! Removing and rebinding."));
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UBTTask_HorrorCreatureAttack::HandleAttackMontageEnded);
			}

			AnimInst->OnMontageEnded.AddDynamic(this, &UBTTask_HorrorCreatureAttack::HandleAttackMontageEnded);
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_HorrorCreatureAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Delegate remove
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn()))
		{
			if (UAnimInstance* AnimInst = AIPawn->GetMesh()->GetAnimInstance())
			{
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UBTTask_HorrorCreatureAttack::HandleAttackMontageEnded);
			}
		}
	}
}

void UBTTask_HorrorCreatureAttack::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent();
	if (!Blackboard) return;

	if (!CachedHorrorCreature->GetSwallowedPlayer())  // If swallowed, the state does not change
	{
		Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, (uint8)EMonsterState::Chase);
	}
	else
	{
		Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, (uint8)EMonsterState::Flee);
	}
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
