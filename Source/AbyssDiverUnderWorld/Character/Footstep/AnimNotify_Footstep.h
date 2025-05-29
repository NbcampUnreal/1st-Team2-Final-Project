// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Footstep.generated.h"

enum class EFootSide : uint8;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:

	/** 현재 발자국의 왼발, 오른발 구분 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footstep")
	EFootSide FootType;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
