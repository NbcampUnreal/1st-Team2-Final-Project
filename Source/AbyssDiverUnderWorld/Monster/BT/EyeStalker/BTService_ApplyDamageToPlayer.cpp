#include "Monster/BT/EyeStalker/BTService_ApplyDamageToPlayer.h"

#include "Monster/Monster.h"
#include "Monster/MonsterAIController.h"

#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/UnderwaterEffectComponent.h"

#include "UI/Flipbooks/FlipbookWidget.h"
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

	// 플레이어가 공격 대상으로 지정이 되어있지 않을 경우 추가
	if (AttackerByVictimMap.Contains(Player) == false)
	{
		AttackerByVictimMap.Add(Player, TaskMemory->Monster);
	}
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

	// 현재 몬스터가 플레이어를 추적하고 있는데 그 플레이어가 새로 타겟이된 플레이어와 일치하지 않을 경우
	// 만약 그 새로운 플레이어가 이동 중이라면 공격을 유지, 그렇지 않으면 타겟 초기화
	if(TaskMemory->ChasingPlayer.IsValid() && TaskMemory->ChasingPlayer.Get() != Player)
	{
		float CurrentPlayerSpeed = Player->GetVelocity().Size();
		
		if (CurrentPlayerSpeed >= RecognizationSpeed)
		{
			TaskMemory->bIsDamageApplied = true;
		}
		else
		{
			TaskMemory->bIsDamageApplied = false;
			TaskMemory->Monster->SetTarget(nullptr);
		}
	}

	TaskMemory->ChasingPlayer = Player;

	// 플레이어가 공격 대상으로 지정이 되어있지 않을 경우 추가
	if (AttackerByVictimMap.Contains(Player) == false)
	{
		AttackerByVictimMap.Add(Player, TaskMemory->Monster);
	}

	TWeakObjectPtr<AMonster>* AttackingMonster = AttackerByVictimMap.Find(Player);

	// 해당 플레이어를 공격하고 있는 몬스터와 자신(몬스터)가 일치하지 않으면 자신은 그 플레이어에게 데미지를 주지 않음
	if (AttackingMonster && AttackingMonster->IsValid() && AttackingMonster->Get() != TaskMemory->Monster)
	{
		return;
	}

	Player->SetIsAttackedByEyeStalker(true);

	// 공격을 시작할 준비, 처음 1 틱은 공격하지 않음.
	// 그렇지 않으면 다수의 EyeStalker가 있을 때 이동하며 시야에서 차례로 벗어날 경우 연속적인 공격이 들어와서..
	if (TaskMemory->bIsDamageApplied == false)
	{
		UUnderwaterEffectComponent* EffectComp = Player->GetEffectComponent();
		if (IsValid(EffectComp))
		{
			const float EffectPlayTime = 0.2f;
			EffectComp->C_PlayFlipbookEffect(EFlipbookType::EyeStalkerGaze, true, EffectPlayTime);
		}

		TaskMemory->bIsDamageApplied = true;
		return;
	}

	UGameplayStatics::ApplyDamage(
		Player,
		AttackPower,
		TaskMemory->AIController.Get(),
		TaskMemory->Monster.Get(),
		UDamageType::StaticClass()
	);

	//FString MonsterName = TaskMemory->Monster.Get()->GetName();
	//TCHAR Last = MonsterName[MonsterName.Len() - 1];
	//int32 TempKey = FCString::Atoi(&Last);

	//FColor Color = FColor(255 - (TempKey) % 256, 255 - (2 * TempKey) % 256, 255 - (5 * TempKey) % 256);

	//GEngine->AddOnScreenDebugMessage(TempKey, 5, Color,
	//	FString::Printf(TEXT("%s Damaged By %s, Damage : %f, PlayerHealth : %d"), *Player->GetName(), *MonsterName, AttackPower, Player->GetStatComponent()->GetCurrentHealth()));
}

void UBTService_ApplyDamageToPlayer::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	
	FApplyDamageToPlayerMemory* TaskMemory = (FApplyDamageToPlayerMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = TaskMemory->ChasingPlayer.Get();
	if (!IsValid(Player) || !Player->IsAlive()) return;

	Player->SetIsAttackedByEyeStalker(false);
	TaskMemory->bIsDamageApplied = false;

	TWeakObjectPtr<AMonster>* AttackingMonster = AttackerByVictimMap.Find(Player);
	// AttackingMonster(해당 플레이어를 공격하고 있는 몬스터)가 유효할 때 
	// 자신(몬스터)이 유효하지 않거나 공격자와 자신이 같으면 노드가 종료되었으니 AttackerByVictimMap에서 지운다.
	if (AttackingMonster && AttackingMonster->IsValid() && (TaskMemory->Monster.IsValid() == false || AttackingMonster->Get() == TaskMemory->Monster))
	{
		AttackerByVictimMap.Remove(Player);
	}

	TaskMemory->ChasingPlayer = nullptr;
}
