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
const FName AEnhancedBossAIController::bIsPlayerHiddenKey = "bIsPlayerHidden";
const FName AEnhancedBossAIController::BloodOccurredLocationKey = "BloodOccurredLocation";

AEnhancedBossAIController::AEnhancedBossAIController()
{
	bIsDetectedBlood = false;
	bIsDetectedPlayer = false;
	bIsDamagedByPlayer = false;
	bCanTransitionToDamagedState = false;
	bIsDisappearPlayer = false;
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

void AEnhancedBossAIController::InitVariables()
{
	// 타겟 플레이어 망각
	Boss->InitTarget();

	// 타겟 사라짐 변수 초기화
	bIsDisappearPlayer = false;

	// 블랙보드 값 초기화
	BlackboardComponent->SetValueAsBool(bIsChasingKey, false);
	BlackboardComponent->SetValueAsBool(bHasDetectedPlayerKey, false);
	BlackboardComponent->SetValueAsBool(bHasAttackedKey, false);
	BlackboardComponent->SetValueAsBool(bIsPlayerHiddenKey, false);
	BlackboardComponent->SetValueAsBool(bHasSeenPlayerKey, false);
	SetBlackboardPerceptionType(EPerceptionType::None);
}

void AEnhancedBossAIController::OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus)
{
	// --------------------- 시각 자극 ---------------------
	// 플레이어가 시야각에 들어오는 경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{	
		// 감지한 대상이 플레이어가 아닌 경우 얼리 리턴
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
		if (!IsValid(Player)) return;

		// 플레이어가 사망 상태인 경우 얼리 리턴
		if (Player->GetCharacterState() == ECharacterState::Death)
		{
			return;
		}

		// 전에 감지한 플레이어와 다른 경우 얼리 리턴
		if (IsValid(Boss->GetTarget()))
		{
			if (Player != Boss->GetTarget()) return;
		}
		
		if (Stimulus.WasSuccessfullySensed())
		{
			OnSightPerceptionSuccess(Player);
		}
		else
		{
			OnSightPerceptionFail();
		}
	}

	// --------------------- 촉각 자극 ---------------------
	// 데미지를 받는 경우
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnDamagePerceptionSuccess();
		}
	}
	
	// --------------------- 청각 자극 ---------------------
	// 플레이어가 피를 흘리는 경우
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnHearingPerceptionSuccess(Stimulus);
		}
	}
}

void AEnhancedBossAIController::OnSightPerceptionSuccess(AUnderwaterCharacter* Player)
{
	// 플레이어가 부쉬에 숨은 상태라면 얼리 리턴
	if (Player->IsHideInSeaweed())	return;
	
	// 블랙보드 키 값 세팅
	BlackboardComponent->SetValueAsBool(bIsChasingKey, true);
	BlackboardComponent->SetValueAsBool(bHasSeenPlayerKey, true);
	SetBlackboardPerceptionType(EPerceptionType::Player);
			
	// 타겟 세팅
	Boss->SetTarget(Player);
	Boss->SetCachedTarget(Player);

	// 플레이어 사라짐 인지 변수 초기화
	bIsDisappearPlayer = false;
}

void AEnhancedBossAIController::OnSightPerceptionFail()
{
	// 플레이어 사라짐 인지 변수 초기화
	bIsDisappearPlayer = true;
	BlackboardComponent->SetValueAsBool(bHasSeenPlayerKey, false);
}

void AEnhancedBossAIController::OnHearingPerceptionSuccess(const FAIStimulus& Stimulus)
{
	// 플레이어가 피를 흘린 위치를 저장
	BlackboardComponent->SetValueAsVector(BloodOccurredLocationKey, Stimulus.StimulusLocation);
	SetBlackboardPerceptionType(EPerceptionType::Blood);
}

void AEnhancedBossAIController::OnDamagePerceptionSuccess()
{
	SetBlackboardPerceptionType(EPerceptionType::Damage);
}
