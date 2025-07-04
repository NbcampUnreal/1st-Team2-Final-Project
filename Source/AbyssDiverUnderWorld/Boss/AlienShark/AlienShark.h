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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;

};
