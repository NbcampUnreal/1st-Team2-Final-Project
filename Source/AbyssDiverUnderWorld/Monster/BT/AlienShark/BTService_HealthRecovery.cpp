#include "Monster/BT/AlienShark/BTService_HealthRecovery.h"

#include "Monster/Monster.h"
#include "Monster/MonsterAIController.h"
#include "Character/StatComponent.h"

UBTService_HealthRecovery::UBTService_HealthRecovery()
{
	NodeName = TEXT("Health Recovery");
	bNotifyBecomeRelevant = true;
}

void UBTService_HealthRecovery::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTHealthRecoveryMemory* TaskMemory = (FBTHealthRecoveryMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	const float MaxHealth = TaskMemory->Monster->GetStatComponent()->MaxHealth;
	const float CurrentHealth = TaskMemory->Monster->GetStatComponent()->CurrentHealth;

	// 체력이 가득 찬 경우 틱 비활성화
	if (CurrentHealth >= MaxHealth)
	{
		// 아주 긴 시간으로 설정하여 틱을 사실상 비활성화
		SetNextTickTime(NodeMemory, 999999.0f);
	}
	else
	{
		// 정상적인 틱 간격으로 설정
		SetNextTickTime(NodeMemory, Interval);
	}
}

void UBTService_HealthRecovery::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	FBTHealthRecoveryMemory* TaskMemory = (FBTHealthRecoveryMemory*)NodeMemory;
	if (!TaskMemory) return;

	const float MaxHealth = TaskMemory->Monster->GetStatComponent()->MaxHealth;
	const float CurrentHealth = TaskMemory->Monster->GetStatComponent()->CurrentHealth;

	if (CurrentHealth >= MaxHealth)
	{
		TaskMemory->Monster->GetStatComponent()->CurrentHealth = MaxHealth;
		SetNextTickTime(NodeMemory, 999999.0f);
		return;
	}
    
	const float RecoveryHealth = CurrentHealth + (MaxHealth * RecoveryRate);
	const float NewHealth = FMath::Clamp(RecoveryHealth, 0.0f, MaxHealth);

	TaskMemory->Monster->GetStatComponent()->CurrentHealth = NewHealth;
	
}
