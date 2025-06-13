#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "Engine/TargetPoint.h"
#include "Kraken.generated.h"

class ABattleFieldVolume;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AKraken : public ABoss
{
	GENERATED_BODY()

public:
	AKraken();

protected:
	virtual void BeginPlay() override;
#pragma region Method
public:
	virtual void OnDeath() override;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetEmissiveTransition();
	
private:
	void GetBattleFieldVolume();
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteAttackCollision;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> TakeDownAttackCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> WieldAttackCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> PickAttackCollision;

private:
	UPROPERTY()
	TObjectPtr<ABattleFieldVolume> BattleFieldVolume;
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
