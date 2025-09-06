#include "Monster/Boss/EnhancedBossAIController.h"
#include "AbyssDiverUnderWorld.h"
#include "Container/BlackboardKeys.h"
#include "Boss.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/EPerceptionType.h"
#include "Gimmic/Volume/ObstacleVolume.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

AEnhancedBossAIController::AEnhancedBossAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsDetectedBlood = false;
	bIsDetectedPlayer = false;
	bIsDamagedByPlayer = false;
	bCanTransitionToDamagedState = false;
	bIsDisappearPlayer = false;
	bIsPerceptionDamage = true;
	bIsPerceptionHearing = true;
	bIsPerceptionSight = true;
}

void AEnhancedBossAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// BeginPlay에서 모든 EnhancedAIController가 모든 AObstacleVolume을 찾아 이벤트를 바인딩한다.
	// BeginPlay 호출 시에 약간의 프레임 드랍이 발생할 우려가 있지만 일회성 호출이므로 예외로 처리한다.
	// Tick에서 플레이어가 Overlapped 되었는지 확인하는 방법에 비해 메모리 효율적이다.
	FTimerHandle BindingTimerHandle;
	GetWorldTimerManager().SetTimer(BindingTimerHandle, this, &AEnhancedBossAIController::BindToObstacleComponent, 1.0f, false);
	
}

void AEnhancedBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnhancedBossAIController::InitVariables()
{
	// 타겟 플레이어 망각
	Boss->InitTarget();

	// 타겟 사라짐 변수 초기화
	bIsDisappearPlayer = false;

	// 블랙보드 값 초기화
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, false);
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasDetectedPlayerKey, false);
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasAttackedKey, false);
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsPlayerHiddenKey, false);
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasSeenPlayerKey, false);
	SetBlackboardPerceptionType(EPerceptionType::None);
}

void AEnhancedBossAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지한 대상이 플레이어가 아니거나 사망 상태인 경우 리턴
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!IsValid(Player) || Player->IsDeath() || Player->IsGroggy()) return;

	LOG(TEXT("OnTargetPerceptionUpdatedHandler : %s"), *Player->GetName());

	// --------------------- 시각 자극 ---------------------
	// 플레이어가 시야각에 들어오는 경우
	if (bIsPerceptionSight && Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
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
	else if (bIsPerceptionDamage && Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnDamagePerceptionSuccess(Player);
		}
	}

	// --------------------- 청각 자극 ---------------------
	// 플레이어가 피를 흘리는 경우
	else if (bIsPerceptionHearing && Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		// 현재 추적중인 플레이어가 피를 흘렸다면 무시한다.
		AUnderwaterCharacter* ChasingPlayer = Cast<AUnderwaterCharacter>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
		if (IsValid(ChasingPlayer))
		{
			if (Player == ChasingPlayer) return;
		}

		if (Stimulus.WasSuccessfullySensed())
		{
			OnHearingPerceptionSuccess(Player);
		}
	}
}

void AEnhancedBossAIController::OnSightPerceptionSuccess(AUnderwaterCharacter* Player)
{
	// 플레이어가 부쉬에 숨은 상태라면 얼리 리턴
	if (Player->IsHideInSeaweed())	return;
	
	LOG(TEXT(" Sight Perception Success !"));
	
	// AlienShark와 다른 AI의 동작방식이 다르다.
	// 코드가 난잡해지긴 하지만, 개발속도 차원에서 일단 bool 값으로 대체한다.
	if (bIsAlienShark)
	{
		// 현재 설정된 타겟이 없는 경우에만 블랙보드 값을 설정한다.
		if (!IsValid(BlackboardComponent->GetValueAsObject(BlackboardKeys::TargetPlayerKey)))
		{
			BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsPlayerHiddenKey, false);
			BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Player);	
		}
	}
	else
	{
		// 블랙보드 키 값 세팅
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, true);
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasSeenPlayerKey, true);
		BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Player);
		SetBlackboardPerceptionType(EPerceptionType::Player);
			
		// 타겟 세팅
		Boss->SetTarget(Player);
		Boss->SetCachedTarget(Player);
		
		// 플레이어 사라짐 인지 변수 초기화
		bIsDisappearPlayer = false;	
	}
}

void AEnhancedBossAIController::OnSightPerceptionFail()
{
	LOG(TEXT("Sight Perception Fail !"));

	// 플레이어 사라짐 인지 변수 초기화
	bIsDisappearPlayer = true;
	BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasSeenPlayerKey, false);
}

void AEnhancedBossAIController::OnHearingPerceptionSuccess(AUnderwaterCharacter* Player)
{
	if (!IsValid(GetCharacter())) return;

	LOG(TEXT("Hearing Perception Success !"));
	
	// 실제 수중에서 피를 흘린 경우 상어는 현재 위치에 따라 피를 감지하는 시간이 다를 것이다.
	// 또한 영구적으로 피를 흘린 위치를 기억하는 것이 아닌, 피를 감지한 이후 5초 정도의 유효 시간이 존재한다.
	// 유효시간이 지나거나 피를 흘린 주변 위치에 도달했다면 Normal 상태로 복귀한다.
	
	const FVector BloodOccurredLocation = Player->GetActorLocation();
	BlackboardComponent->SetValueAsVector(BlackboardKeys::BloodOccurredLocationKey, BloodOccurredLocation);

	if (bIsAlienShark)
	{
		BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Player);
		
		const FVector CurrentLocation = GetCharacter()->GetActorLocation();
		const float Distance = FVector::Dist(CurrentLocation, BloodOccurredLocation);

		// 거리가 가까울수록 피를 감지하는 시간이 짧아지고,
		// 거리가 멀수록 피를 감지하는 데에 시간이 오래걸린다.
		constexpr float MinDetectedTime = 1.0f;
		constexpr float MaxDetectedTime = 3.0f;

		// 최대로 감지할 수 있는 거리를 추출한다.
		float MaxHearingDistance = 6000.f;
		if (IsValid(HearingConfig))
		{
			MaxHearingDistance = HearingConfig->HearingRange;
		}

		// 거리에 따라 MinDetectedTime ~ MaxDetectedTime 값으로 보간한다.
		const float NormalizedDistance = FMath::Clamp(Distance / MaxHearingDistance, 0.0f, 1.0f);
		const float DetectedTime = FMath::Lerp(MinDetectedTime, MaxDetectedTime, NormalizedDistance);

		// DetectedTime 이후 플레이어 피 감지 이벤트를 Notify 한다.
		GetWorldTimerManager().SetTimer(BloodDetectedTimerHandle, this, &AEnhancedBossAIController::SetBloodDetectedState, DetectedTime, false);
	}
	else
	{
		SetBlackboardPerceptionType(EPerceptionType::Blood);	
	}
}

void AEnhancedBossAIController::SetBloodDetectedState()
{
	// 피 감지 상태 변수를 활성화한다.
	// 5초동안 피를 감지하지 못 했다면 상태를 초기화한다.
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bIsChasingPlayerKey, true);
}



void AEnhancedBossAIController::OnDamagePerceptionSuccess(AUnderwaterCharacter* Player)
{
	LOG(TEXT("Damage Perception Success !"));
	
	GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, Player);
	
	if (IsValid(Boss))
	{
		Boss->SetTarget(Player);
	}
	
	if (bIsAlienShark)
	{
		GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bIsChasingPlayerKey, true);
	}
	else
	{
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bIsChasingKey, true);
		BlackboardComponent->SetValueAsBool(BlackboardKeys::bHasSeenPlayerKey, true);
	}
}

void AEnhancedBossAIController::OnObstacleComponentBeginOverlap(AUnderwaterCharacter* OverlappedPlayer)
{
	if (!IsValid(OverlappedPlayer)) return;
	
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player)) return;
	
	if (Player == OverlappedPlayer)
	{
		GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bIsChasingPlayerKey, false);
		GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bIsPlayerHiddenKey, false);
		GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, nullptr);
	}
	
}

void AEnhancedBossAIController::BindToObstacleComponent()
{
	for (AObstacleVolume* Volume : TActorRange<AObstacleVolume>(GetWorld()))
	{
		if (IsValid(Volume))
		{
			Volume->OnObstacleComponentBeginOverlapDelegate.AddDynamic(this, &AEnhancedBossAIController::OnObstacleComponentBeginOverlap);
		}
	}
}
