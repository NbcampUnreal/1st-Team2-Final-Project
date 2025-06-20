#include "Boss/Kraken/KrakenAIController.h"
#include "Kraken.h"

void AKrakenAIController::BeginPlay()
{
	Super::BeginPlay();

	SetSightRadius(0.0f);
}

void AKrakenAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

float AKrakenAIController::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	AKraken* Kraken = Cast<AKraken>(GetOwner());
	if (!IsValid(Kraken) || !Kraken->GetCanBattle()) return 0.0f;
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AKrakenAIController::OnSightPerceptionSuccess(AUnderwaterCharacter* Player)
{
	LOG(TEXT("Sight Perception Success !"));
	AKraken* Kraken = Cast<AKraken>(GetCharacter());
	if (!IsValid(Kraken) || !Kraken->GetCanBattle()) return;
	
	Super::OnSightPerceptionSuccess(Player);
	
	const bool bCanDetect = GetBlackboardComponent()->GetValueAsBool("bCanDetect");
	if (bCanDetect)
	{
		// WeakPtr 사용으로 안전한 참조 보장
		TWeakObjectPtr<AKrakenAIController> WeakThis = this;
		
		GetWorldTimerManager().SetTimer(DetectedStateTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid() && IsValid(WeakThis->GetBlackboardComponent()))
			{
				WeakThis->GetBlackboardComponent()->SetValueAsBool("bCanDetect", true);
			}
		}, DetectedStateInterval, false);
	}
}

void AKrakenAIController::OnDamagePerceptionSuccess(AUnderwaterCharacter* Player)
{
	AKraken* Kraken = Cast<AKraken>(GetCharacter());
	if (!IsValid(Kraken) || !Kraken->GetCanBattle()) return;
	
	Super::OnDamagePerceptionSuccess(Player);
}

