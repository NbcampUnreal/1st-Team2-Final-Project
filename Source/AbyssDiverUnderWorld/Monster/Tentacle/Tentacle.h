// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"
#include "Tentacle.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API ATentacle : public AMonster
{
	GENERATED_BODY()
	
public:
	ATentacle();

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack")
	TObjectPtr<USphereComponent> TentacleHitSphere;

#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return TentacleHitSphere; }

#pragma endregion
};
