// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "MonsterSoundComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UMonsterSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMonsterSoundComponent();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	// Idle Loop Sound
	UFUNCTION(Server, Reliable)
	void S_PlayPatrolLoopSound();
	void S_PlayPatrolLoopSound_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayPatrolLoopSound();
	void M_PlayPatrolLoopSound_Implementation();

	// Chase Loop Sound
	UFUNCTION(Server, Reliable)
	void S_PlayChaseLoopSound();
	void S_PlayChaseLoopSound_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayChaseLoopSound();
	void M_PlayChaseLoopSound_Implementation();

	// Stop Loop Sound
	UFUNCTION(Server, Reliable)
	void S_StopAllLoopSound();
	void S_StopAllLoopSound_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_StopAllLoopSound();
	void M_StopAllLoopSound_Implementation();
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, Category = "Sound|Loop")
	TObjectPtr<USoundBase> PatrolLoopSound;

	UPROPERTY(EditAnywhere, Category = "Sound|Loop")
	TObjectPtr<USoundBase> ChaseLoopSound;

private:
	TObjectPtr<UAudioComponent> PatrolLoopComponent;
	TObjectPtr<UAudioComponent> ChaseLoopComponent;
#pragma endregion
};
