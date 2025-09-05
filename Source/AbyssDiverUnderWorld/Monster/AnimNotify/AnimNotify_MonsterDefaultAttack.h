// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MonsterDefaultAttack.generated.h"

class UCapsuleComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_MonsterDefaultAttack : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_MonsterDefaultAttack();

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollisionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsMeshCollision : 1;

private:
	FTimerHandle AttackTimer;
};
