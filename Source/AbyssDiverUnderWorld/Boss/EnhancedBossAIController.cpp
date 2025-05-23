#include "Boss/EnhancedBossAIController.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Enum/EPerceptionType.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

const FName AEnhancedBossAIController::bHasSeenPlayerKey = "bHasSeenPlayer";
const FName AEnhancedBossAIController::PerceptionTypeKey = "EPerceptionType";
const FName AEnhancedBossAIController::bHasDetectedPlayerKey = "bHasDetectedPlayer";
const FName AEnhancedBossAIController::bIsChasingKey = "bIsChasing";
const FName AEnhancedBossAIController::bHasAttackedKey = "bHasAttacked";
const FName AEnhancedBossAIController::BloodOccurredLocationKey = "BloodOccurredLocation";

AEnhancedBossAIController::AEnhancedBossAIController()
{
	bIsDetectedBlood = false;
	bIsDetectedPlayer = false;
	bIsDamagedByPlayer = false;
	bCanTransitionToDamagedState = false;
	bIsDisappearPlayer = false;
	TargetSaveTime = 3.f;
}

void AEnhancedBossAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnhancedBossAIController::OnTargetPerceptionUpdatedHandler);
	}
}

void AEnhancedBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnhancedBossAIController::SetBlackboardPerceptionType(EPerceptionType InPerceptionType)
{
	BlackboardComponent->SetValueAsEnum(PerceptionTypeKey, static_cast<uint8>(InPerceptionType));
}

void AEnhancedBossAIController::InitPerceptionInfo()
{
	SetBlackboardPerceptionType(EPerceptionType::Finish);
	Boss->InitTarget();
	BlackboardComponent->SetValueAsBool(bIsChasingKey, false);
}

void AEnhancedBossAIController::InitVariables()
{
	// 타겟 플레이어 망각
	Boss->InitTarget();
	Boss->InitCachedTarget();

	// 블랙보드 값 초기화
	BlackboardComponent->SetValueAsBool(bIsChasingKey, false);
	BlackboardComponent->SetValueAsBool(bHasDetectedPlayerKey, false);
	BlackboardComponent->SetValueAsBool(bHasAttackedKey, false);
	SetBlackboardPerceptionType(EPerceptionType::None);
}

void AEnhancedBossAIController::OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus)
{
	// Stimulus 조건문은 "감지에 성공할 경우", "시야에서 벗어날 경우" 각각 한 번씩 호출된다.
	// 따라서 Stimulus 조건문 내부에서 플레이어가 해초더미에 있는지 체크하는 건 옳지 않다.
	// 결론적으로 Detected, Chase Task의 TickTask에서 플레이어가 해초더미에 있는지 확인해주어야 한다.
	// 오직 타겟은 한 명으로만 지정한다.



	
	// --------------------- 시각 자극 ---------------------
	// 플레이어가 시야각에 들어오는 경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// 감지한 대상이 플레이어가 아닌 경우 얼리 리턴
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (!IsValid(Player)) return;

		// 전에 감지한 플레이어와 다른 경우 얼리 리턴
		if (IsValid(Boss->GetTarget()))
		{
			if (Player != Boss->GetTarget()) return;
		}
		
		if (Stimulus.WasSuccessfullySensed())
		{
			LOG(TEXT("Stimulus Success !"));
			BlackboardComponent->SetValueAsBool(bIsChasingKey, true);
			BlackboardComponent->SetValueAsBool(bHasSeenPlayerKey, true);
			Boss->SetTarget(Player);
			Boss->SetCachedTarget(Player);
			bIsDisappearPlayer = false;
			SetBlackboardPerceptionType(EPerceptionType::Player);
			GetWorldTimerManager().ClearTimer(TargetSaveTimerHandle);
		}
		else
		{
			LOG(TEXT("Player was out of sight !"));
			bIsDisappearPlayer = true;
			BlackboardComponent->SetValueAsBool(bHasSeenPlayerKey, false);
			GetWorldTimerManager().SetTimer(TargetSaveTimerHandle, this, &AEnhancedBossAIController::InitPerceptionInfo, TargetSaveTime, false);
		}
	}

	// --------------------- 촉각 자극 ---------------------
	// 데미지를 받는 경우
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			SetBlackboardPerceptionType(EPerceptionType::Damage);
		}
	}
	
	// --------------------- 청각 자극 ---------------------
	// 플레이어가 피를 흘리는 경우
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			BlackboardComponent->SetValueAsVector(BloodOccurredLocationKey, Stimulus.StimulusLocation);
			SetBlackboardPerceptionType(EPerceptionType::Blood);
		}
	}
}
