#include "Boss/EyeStalker/EyeStalkerAIController.h"
#include "Character/UnderwaterCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

void AEyeStalkerAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEyeStalkerAIController::OnSightPerceptionUpdatedHandler);
	}
}

void AEyeStalkerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEyeStalkerAIController::OnSightPerceptionSuccess(AUnderwaterCharacter* Player)
{
	if (TargetPlayers.Num() == 0)
	{
		GetBlackboardComponent()->SetValueAsObject("TargetPlayer", Player);
	}

	TargetPlayers.Emplace(Player);
}

void AEyeStalkerAIController::OnSightPerceptionFail(AUnderwaterCharacter* Player)
{
	TargetPlayers.Remove(Player);
	
	if (TargetPlayers.Num() == 0)
	{
		GetBlackboardComponent()->SetValueAsBool("bHasDetected", false);
		GetBlackboardComponent()->SetValueAsObject("TargetPlayer", nullptr);	
	}
	else
	{
		SetRandomTargetPlayer();
	}
}

void AEyeStalkerAIController::OnSightPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지한 대상이 플레이어가 아니거나 사망 상태인 경우 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!IsValid(Player) || Player->GetCharacterState() == ECharacterState::Death) return;

	LOG(TEXT("OnTargetPerceptionUpdatedHandler : %s"), *Player->GetName());
	
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnSightPerceptionSuccess(Player);
		}
		else
		{
			OnSightPerceptionFail(Player);
		}
	}
}

void AEyeStalkerAIController::SetRandomTargetPlayer()
{
	// 랜덤 인덱스 생성
	uint8 RandomIndex = FMath::RandRange(0, TargetPlayers.Num() - 1);
   
	// 선택된 플레이어를 블랙보드에 설정
	AUnderwaterCharacter* SelectedPlayer = TargetPlayers[RandomIndex];
	GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SelectedPlayer);
}
