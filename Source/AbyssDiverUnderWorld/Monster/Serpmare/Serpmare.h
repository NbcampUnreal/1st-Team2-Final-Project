﻿#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"

#include "Serpmare.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASerpmare : public AMonster
{
	GENERATED_BODY()

public:
	ASerpmare();

protected:

	virtual void BeginPlay() override;

public:

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:


	virtual void Attack() override;

	virtual void NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor) override;

	virtual void AddDetection(AActor* Actor) override;

	virtual void RemoveDetection(AActor* Actor) override;

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
