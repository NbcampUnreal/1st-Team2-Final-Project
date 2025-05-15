// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_DefaultAttack.h"
#include "Monster/Monster.h"
#include "AIController.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = "DefaultAttack";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_DefaultAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	if (!AIPawn) return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIPawn);
	if (!Monster || !Monster->GetAttackMontage()) return EBTNodeResult::Failed;

	// In Server
	if (Monster->HasAuthority())
	{
		// Sync to Client
		Monster->M_PlayAttackMontage();

		// Detect end of montage ¡æ End BTTask
		if (UAnimInstance* AnimInst = AIPawn->GetMesh()->GetAnimInstance())
		{
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
		FinishLatentTask(*CachedOwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}
