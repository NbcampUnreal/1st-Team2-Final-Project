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

	// Flee Loop Sound
	UFUNCTION(Server, Reliable)
	void S_PlayFleeLoopSound();
	void S_PlayFleeLoopSound_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayFleeLoopSound();
	void M_PlayFleeLoopSound_Implementation();

	// HitReact Sound
	UFUNCTION(Server, Reliable)
	void S_PlayHitReactSound();
	void S_PlayHitReactSound_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayHitReactSound();
	void M_PlayHitReactSound_Implementation();

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

	UPROPERTY(EditAnywhere, Category = "Sound|Loop")
	TObjectPtr<USoundBase> FleeLoopSound;

	UPROPERTY(EditAnywhere, Category = "Sound|HitReact")
	TObjectPtr<USoundBase> HitReactSound;

private:
	TObjectPtr<UAudioComponent> PatrolLoopComponent;
	TObjectPtr<UAudioComponent> ChaseLoopComponent;
	TObjectPtr<UAudioComponent> FleeLoopComponent;
#pragma endregion
};
