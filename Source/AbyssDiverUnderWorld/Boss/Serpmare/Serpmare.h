#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "Serpmare.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASerpmare : public ABoss
{
	GENERATED_BODY()

public:
	ASerpmare();

public:
	virtual void BeginPlay() override;

protected:
	virtual void Attack() override;

private:
	void InitAttackInterval();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> AppearAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> DisappearAnimation;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	float AttackInterval = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> LowerBodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> WeakPointMesh;

private:
	FTimerHandle AttackIntervalTimer;
	uint8 bCanAttack : 1 = true;

public:
	uint8 GetCanAttack() const { return bCanAttack; }
};
