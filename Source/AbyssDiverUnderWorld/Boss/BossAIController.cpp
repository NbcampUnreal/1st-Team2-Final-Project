#include "Boss/BossAIController.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss.h"
#include "EBossState.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName ABossAIController::BossStateKey = "BossState";

ABossAIController::ABossAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

	DefaultVisionAngle = 90.0f;
	ChasingVisionAngle = 150.0f;

	// 시야 설정
	SightConfig->SightRadius = 5000.0f;					// 시야 범위
	SightConfig->LoseSightRadius = 6000.0f;				// 시야 상실 범위
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;	// 주변 시야 각도
	SightConfig->SetMaxAge(5.0f);						// 자극 최대 기억 시간

	// 감지 설정
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	// 시각을 우선순위로 설정
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	bIsDetectedStatePossible = true;
	AccumulatedTime = 0.0f;
	DetectedStateInterval = 20.0f;
}

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABossAIController::OnTargetPerceptionUpdated);
	}
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsValid(BehaviorTree))
	{
		UseBlackboard(BehaviorTree->BlackboardAsset, (UBlackboardComponent*&)BlackboardComponent);
		RunBehaviorTree(BehaviorTree);
	}
}

void ABossAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Detected 상태로 전이한지 DetectedStateInterval 만큼 지났는지 확인
	if (!bIsDetectedStatePossible)
	{
		AccumulatedTime += DeltaSeconds;

		if (AccumulatedTime >= DetectedStateInterval)
		{
			bIsDetectedStatePossible = true;
			AccumulatedTime = 0.0f;
		}
	}
}

void ABossAIController::SetVisionAngle(float& Angle)
{
	// 시야각 전환 함수
	UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass())));
	SightConfigInstance->PeripheralVisionAngleDegrees = Angle;
	AIPerceptionComponent->ConfigureSense(*SightConfigInstance);
}

void ABossAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// @TODO: 감지된 Actor를 캐릭터로 캐스팅해서 검사하는 로직 구현 필요
	if (!IsValid(Actor)) return;

	// 시각 자극인지 확인
	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		M_AddDetectedPlayer(Actor);
	}
	else
	{
		M_RemoveDetectedPlayer(Actor);
	}
}

void ABossAIController::M_AddDetectedPlayer_Implementation(AActor* Target)
{
	ABoss* Boss = Cast<ABoss>(GetCharacter());
	if (!IsValid(Boss)) return;

	APawn* Player = Cast<APawn>(Target);
	if (IsValid(Player))
	{
		LOG(TEXT(" [AI] Target Perception Updated: %s"), *Player->GetName());
		Boss->SetTarget((Player));
	}

	// 처음으로 플레이어를 감지한 경우 조건에 따라 Detected | Chase 상태로 전이
	if (DetectedPlayers.Num() <= 0)
	{
		if (bIsDetectedStatePossible)
		{
			BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Detected));
			bIsDetectedStatePossible = false;
		}
		else
		{
			BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Chase));
		}
	}

	// 감지된 플레이어 리스트에 추가
	DetectedPlayers.Add(Target);
}

void ABossAIController::M_RemoveDetectedPlayer_Implementation(AActor* Target)
{
	// 감지된 플레이어 리스트에서 제거
	DetectedPlayers.Remove(Target);

	// 감지한 플레이어가 리스트에서 모두 제거된 경우 Investigate 상태로 전이
	if (DetectedPlayers.Num() <= 0)
	{
		ABoss* Boss = Cast<ABoss>(GetCharacter());
		if (!IsValid(Boss)) return;
		
		Boss->SetLastDetectedLocation(Target->GetActorLocation());
		BlackboardComponent->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Investigate));
	}
}
