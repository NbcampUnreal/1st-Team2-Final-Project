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

protected:
	virtual void BeginPlay() override;

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TentacleHitSphere;

#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return TentacleHitSphere; }

#pragma endregion
};
