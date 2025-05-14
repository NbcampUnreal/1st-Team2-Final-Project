#include "Boss/BossAIController.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss.h"
#include "EBossState.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName ABossAIController::BossStateKey = "BossState";

ABossAIController::ABossAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

	MoveToLocationAcceptanceRadius = 50.0f;
	MoveToActorAcceptanceRadius = 500.0f;
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

	// 시야 감지 가능한 상태로 설정
	bIsSightDetectionPossible = true;
}

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AIPerceptionComponent) && bIsSightDetectionPossible)
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

	Boss = Cast<ABoss>(InPawn);
}

void ABossAIController::SetVisionAngle(float& Angle)
{
	// Angle 수치로 시야각 전환
	UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass())));
	SightConfigInstance->PeripheralVisionAngleDegrees = Angle;
	AIPerceptionComponent->ConfigureSense(*SightConfigInstance);
}

void ABossAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지한 대상이 플레이어가 아닌 경우 얼리 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!IsValid(Player)) return;

	// 시각 자극인지 확인
	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) return;
	
	// 감지에 성공 했다면 플레이어 감지 리스트에 추가
	if (Stimulus.WasSuccessfullySensed())
	{
		//LOGN(TEXT("[Detected] %s"), *Player->GetName());
		M_AddDetectedPlayer(Player);
	}
	// 시야각에서 벗어났다면 플레이어 감지 리스트에서 제거
	else
	{
		//LOGN(TEXT("[Lost Sight] %s"), *Player->GetName());
		M_RemoveDetectedPlayer(Player);
	}
}

void ABossAIController::M_AddDetectedPlayer_Implementation(AUnderwaterCharacter* Target)
{
	if (!IsValid(Target)) return;

	// 감지된 플레이어 리스트에 추가
	DetectedPlayers.Add(Target);
	
	// 이미 추적중인 플레이어가 있는 경우 얼리 리턴
	if (IsValid(Boss->GetTarget()))
	{
		//LOGN(TEXT("[Already Detected] %s"), *Boss->GetTarget()->GetName());
		return;
	}

	// 최초로 플레이어를 감지한 경우 추적할 타겟으로 설정
	Boss->SetTarget(Target);

	// Detected 상태로 전이된 지 DetectedStateInterval이 지난 경우 Detected 상태로 전이
	if (bIsDetectedStatePossible)
	{
		Boss->SetBossState(EBossState::Detected);
		bIsDetectedStatePossible = false;
		GetWorldTimerManager().SetTimer(DetectedStateTimerHandle, this, &ABossAIController::SetDetectedStatePossible, DetectedStateInterval, false);
	}
	// 만약 아니라면 Chase 상태로 전이
	else
	{
		Boss->SetBossState(EBossState::Chase);
	}
}

void ABossAIController::M_RemoveDetectedPlayer_Implementation(AUnderwaterCharacter* Target)
{
	if (!IsValid(Target)) return;
	
	// 감지된 플레이어 리스트에서 제거
	DetectedPlayers.Remove(Target);

	// 시야각에서 사라진 타겟이 추적중이었던 플레이어인 경우 Investigate 상태로 전이
	if (Target == Boss->GetTarget())
	{
		Boss->SetLastDetectedLocation(Target->GetActorLocation());
		Boss->SetBossState(EBossState::Investigate);
		Boss->InitTarget();
	}
}

EPathFollowingRequestResult::Type ABossAIController::MoveToActorWithRadius()
{
	return MoveToActor(Boss->GetTarget(), MoveToActorAcceptanceRadius);
}

EPathFollowingRequestResult::Type ABossAIController::MoveToLocationWithRadius(const FVector& Location)
{
	return MoveToLocation(Location, MoveToLocationAcceptanceRadius);
}

void ABossAIController::SetDetectedStatePossible()
{
	bIsDetectedStatePossible = true;
}