#include "Boss/Task/EyeStalker/BTService_ApplyDamageToPlayer.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"
#include "Boss/EyeStalker/EyeStalkerAIController.h"
#include "Character/UnderwaterCharacter.h"
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

	TaskMemory->AIController = Cast<AEyeStalkerAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player) || !Player->IsAlive()) return;
	
}

void UBTService_ApplyDamageToPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player) || Player->IsDeath() || Player->IsGroggy()) return;

	if (!TaskMemory->bIsDamageApplied && Player->IsAttackedByEyeStalker()) return;

	Player->SetIsAttackedByEyeStalker(true);
	TaskMemory->bIsDamageApplied = true;

	UGameplayStatics::ApplyDamage(
		Player,
		AttackPower,
		TaskMemory->AIController.Get(),
		TaskMemory->Boss.Get(),
		UDamageType::StaticClass()
	);
}

void UBTService_ApplyDamageToPlayer::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player) || !Player->IsAlive()) return;

	if (!TaskMemory->bIsDamageApplied && Player->IsAttackedByEyeStalker()) return;
	
	Player->SetIsAttackedByEyeStalker(false);
	TaskMemory->bIsDamageApplied = false;
}
