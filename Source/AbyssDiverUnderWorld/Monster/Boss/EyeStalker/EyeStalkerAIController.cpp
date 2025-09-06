#include "Monster/Boss/EyeStalker/EyeStalkerAIController.h"
#include "Container/BlackboardKeys.h"

#include "Monster/Effect/PostProcessSettingComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

void AEyeStalkerAIController::BeginPlay()
{
	Super::BeginPlay();
	
	/*if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEyeStalkerAIController::OnSightPerceptionUpdatedHandler);
	}*/
}

//void AEyeStalkerAIController::OnPossess(APawn* InPawn)
//{
//	Super::OnPossess(InPawn);
//}
//
void AEyeStalkerAIController::InitTargetPlayer()
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player)) return;

	UPostProcessSettingComponent* PostProcessSettingComponent = Player->GetPostProcessSettingComponent();
	if (!IsValid(PostProcessSettingComponent)) return;

	Player->SetIsAttackedByEyeStalker(false);
	PostProcessSettingComponent->C_DeactivateVignetteEffect();
	
	GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, nullptr);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bIsAttackingKey, false);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bHasDetectedKey, false);
}
//
//void AEyeStalkerAIController::RemoveTargetPlayer(AUnderwaterCharacter* Player)
//{
//	if (TargetPlayers.Contains(Player))
//	{
//		TargetPlayers.Remove(Player);
//	}
//}

//void AEyeStalkerAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
//{
//	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
//}

//void AEyeStalkerAIController::OnSightPerceptionSuccess(AUnderwaterCharacter* Player)
//{
//	if (TargetPlayers.Num() == 0)
//	{
//		GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Player);
//	}
//
//	if (!TargetPlayers.Contains(Player))
//	{
//		TargetPlayers.Emplace(Player);
//	}
//}
//
//void AEyeStalkerAIController::OnSightPerceptionFail(AUnderwaterCharacter* Player)
//{
//	AUnderwaterCharacter* TargetPlayer = Cast<AUnderwaterCharacter>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
//
//	if (IsValid(TargetPlayer) && TargetPlayer == Player)
//	{
//		InitTargetPlayer();
//	}
//	
//	RemoveTargetPlayer(Player);
//	
//	if (TargetPlayers.Num() == 0)
//	{
//		InitTargetPlayer();
//	}
//	else
//	{
//		SetRandomTargetPlayer();
//	}
//}
//
//void AEyeStalkerAIController::OnSightPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus)
//{
//	// 감지한 대상이 플레이어가 아니거나 사망 상태인 경우 리턴
//	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
//	if (!IsValid(Player)) return;
//	
//	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
//	{
//		if (Stimulus.WasSuccessfullySensed())
//		{
//			OnSightPerceptionSuccess(Player);
//		}
//		else
//		{
//			OnSightPerceptionFail(Player);
//		}
//	}
//}
//
//void AEyeStalkerAIController::SetRandomTargetPlayer()
//{
//	RemoveInValidTargetPlayers();
//	
//	// 랜덤 인덱스 생성
//	const uint8 Tries = TargetPlayers.Num() * 3;
//	for (uint8 i=0; i<Tries; i++)
//	{
//		const uint8 RandomIndex = FMath::RandRange(0, TargetPlayers.Num() - 1);
//		AUnderwaterCharacter* SelectedPlayer = TargetPlayers[RandomIndex];
//
//		if (SelectedPlayer->IsGroggy() || SelectedPlayer->IsDeath())
//		{
//			// 선택된 플레이어가 기절 상태이거나 사망 상태인 경우, 다음 플레이어를 선택
//			continue;
//		}
//		else
//		{
//			// 선택된 플레이어를 블랙보드에 설정
//			GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, SelectedPlayer);
//			break;	
//		}
//	}
//}
//
//void AEyeStalkerAIController::RemoveInValidTargetPlayers()
//{
//	for (int32 i = TargetPlayers.Num() - 1; i >= 0; --i)
//	{
//		AUnderwaterCharacter* Player = TargetPlayers[i];
//		if (!IsValid(Player))
//		{
//			TargetPlayers.RemoveAt(i);
//		}
//	}
//}
