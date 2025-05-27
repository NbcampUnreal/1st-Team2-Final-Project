// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"
#include "GobleFish.generated.h"

class AGFProjectile;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AGobleFish : public AMonster
{
	GENERATED_BODY()
	

public:
	AGobleFish();

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void FireProjectile();

	virtual void PlayAttackMontage() override;
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> GobleFishHitSphere;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AGFProjectile> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|AttackAnimation", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> RangedAttackAnimations;
#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return GobleFishHitSphere; }

#pragma endregion
};
