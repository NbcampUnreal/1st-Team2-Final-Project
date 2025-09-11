#pragma once

#include "CoreMinimal.h"
#include "Monster/Boss/Boss.h"
#include "Blowfish.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABlowfish : public AMonster
{
	GENERATED_BODY()

public:
	ABlowfish();

public:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnDeath() override;

	UFUNCTION(NetMulticast, Reliable)
	void M_TriggerExplosion();
	void M_TriggerExplosion_Implementation();

	void Explosion();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ScaleUpTriggered();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ExplosionTriggeredMovementMultiplier;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ExplosionHealthRatio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ExplosionDelayTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float ExplosionRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float ExplosionDamage = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> TailCollision;

private:
	uint8 bIsExplosionTriggered : 1;
	FTimerHandle ExplosionTimerHandle;
};
