#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "Engine/TargetPoint.h"
#include "Kraken.generated.h"

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
	virtual void Move() override;
	virtual void MoveStop() override;
	virtual void MoveToTarget() override;
	virtual void MoveToLastDetectedLocation() override;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void FlyingMoveToTarget(const FVector& TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
	void FlyingMoveToTargetStop();
	
private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Boss|PatrolPoints")
	TArray<TObjectPtr<ATargetPoint>> PatrolPoints;

private:
	uint8 CurrentPatrolPointIndex = 0;
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
