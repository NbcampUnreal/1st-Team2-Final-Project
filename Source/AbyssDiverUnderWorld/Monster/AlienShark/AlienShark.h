#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"

#include "AlienShark.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AAlienShark : public AMonster
{
	GENERATED_BODY()

public:
	AAlienShark();

public:
	virtual void BeginPlay() override;

protected:

	virtual void NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;
	
};