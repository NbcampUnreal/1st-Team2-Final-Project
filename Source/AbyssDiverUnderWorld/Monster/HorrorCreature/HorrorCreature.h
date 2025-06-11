// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"
#include "HorrorCreature.generated.h"

class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AHorrorCreature : public AMonster
{
	GENERATED_BODY()
	

public:
	AHorrorCreature();
	
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION()
	void SwallowPlayer(AUnderwaterCharacter* Victim);
	UFUNCTION()
	void EjectPlayer(AUnderwaterCharacter* Victim);

protected:
	UFUNCTION()
	void OnSwallowTriggerOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> HorrorCreatureHitSphere;
	UPROPERTY()
	TObjectPtr<AUnderwaterCharacter> SwallowedPlayer;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> EjectMontage;

private:
	UPROPERTY(EditAnywhere, Category = "Lanch")
	float LanchStrength;

	uint8 bCanSwallow : 1;
#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return HorrorCreatureHitSphere; }
	AUnderwaterCharacter* GetSwallowedPlayer() { return SwallowedPlayer; }

#pragma endregion
};
