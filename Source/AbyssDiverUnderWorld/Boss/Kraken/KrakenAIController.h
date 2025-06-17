#pragma once

#include "CoreMinimal.h"
#include "Boss/EnhancedBossAIController.h"
#include "KrakenAIController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AKrakenAIController : public AEnhancedBossAIController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void OnSightPerceptionSuccess(AUnderwaterCharacter* Player) override;
	virtual void OnDamagePerceptionSuccess(AUnderwaterCharacter* Player) override;
	
};
