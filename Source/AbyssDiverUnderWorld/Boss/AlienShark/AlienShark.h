#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "AlienShark.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AAlienShark : public ABoss
{
	GENERATED_BODY()

public:
	AAlienShark();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	bool HasObstacleAhead();
	void StartTurn();
	void PerformTurn(const float& DeltaTime);
	void PerformNormalMovement(float DeltaTime);
	void SetNewTarget();
	void SmoothMoveAlongSurface(const float& InDeltaTime);
	FVector GetRandomNavMeshLocation(const FVector& Origin, const float& Radius) const;
	void ReturnToNavMeshArea();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TraceDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float RotationSpeed = 1.f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WanderRadius = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinTargetDistance = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float NavMeshCheckInterval = 0.5f;

private:
	FCollisionQueryParams Params;
	const int32 MaxIterations = 6;
	const float AngleStep = 30.0f;
	const float FourDirectionTraceDistance = 200.0f;
	uint8 bHasTarget : 1 = false;
	uint8 bIsTurning : 1 = false;
	FVector TurnDirection;
	float AcceptanceRadius = 10.f;
	float TurnTimer = 0.0f;
	float NavMeshCheckTimer = 0.0f;

};
