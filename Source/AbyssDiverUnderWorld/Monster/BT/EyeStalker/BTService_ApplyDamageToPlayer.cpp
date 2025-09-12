#include "Monster/BT/EyeStalker/BTService_ApplyDamageToPlayer.h"

#include "Monster/Monster.h"
//#include "Monster/Boss/EnhancedBossAIController.h"
#include "Monster/MonsterAIController.h"

#include "Character/UnderwaterCharacter.h"
#include "Container/BlackboardKeys.h"

#include "Kismet/GameplayStatics.h"

UBTService_ApplyDamageToPlayer::UBTService_ApplyDamageToPlayer()
{
	NodeName = TEXT("Apply Damage To Player");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bCreateNodeInstance = false;
}

void UBTService_ApplyDamageToPlayer::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Monster.IsValid()) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player) || !Player->IsAlive()) return;
	
}

void UBTService_ApplyDamageToPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player))
	{
		return;
	}

	if (Player->IsDeath() || Player->IsGroggy())
	{
		Player->OnUntargeted(TaskMemory->Monster.Get());
		return;
	}

	if (!TaskMemory->bIsDamageApplied && Player->IsAttackedByEyeStalker()) return;

	Player->SetIsAttackedByEyeStalker(true);
	TaskMemory->bIsDamageApplied = true;

	UGameplayStatics::ApplyDamage(
		Player,
		AttackPower,
		TaskMemory->AIController.Get(),
		TaskMemory->Monster.Get(),
		UDamageType::StaticClass()
	);
}

void UBTService_ApplyDamageToPlayer::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player) || !Player->IsAlive()) return;

	if (!TaskMemory->bIsDamageApplied && Player->IsAttackedByEyeStalker()) return;
	
	Player->SetIsAttackedByEyeStalker(false);
	TaskMemory->bIsDamageApplied = false;
}
