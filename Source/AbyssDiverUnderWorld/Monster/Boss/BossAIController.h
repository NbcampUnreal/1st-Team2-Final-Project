#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Monster/MonsterAIController.h"
#include "BossAIController.generated.h"

class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class AUnderwaterCharacter;
class ABoss;
struct FAIStimulus;
class UAISenseConfig_Sight;
class UPawnSensingComponent;
class UBehaviorTreeComponent;

enum class EMonsterState : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABossAIController : public AMonsterAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

#pragma region Method
public:

	void SetSightRadius(float InRadius);
	void SetVisionAngle(float Angle);

private:
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DefaultVisionAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float ChasingVisionAngle;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float DetectedStateInterval;

	UPROPERTY()
	TObjectPtr<ABoss> Boss;

	FTimerHandle DetectedStateTimerHandle;

};