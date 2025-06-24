// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlaySoundWithDucking.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_PlaySoundWithDucking : public UAnimNotify
{
	GENERATED_BODY()
	

public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#pragma region Variable
	UPROPERTY(EditAnywhere)
	USoundBase* SoundToPlay;

	// Reduced loudness when another sound arrives
	UPROPERTY(EditAnywhere)
	float DuckVolume = 0.2;

	UPROPERTY(EditAnywhere)
	float DuckDuration = 0.3;

	UPROPERTY(EditAnywhere)
	float RecoverDuration = 0.3;

#pragma endregion
};
