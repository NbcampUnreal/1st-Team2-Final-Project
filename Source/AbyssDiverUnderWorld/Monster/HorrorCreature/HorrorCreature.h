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
	virtual void Tick(float DeltaTime) override;

#pragma region Method
public:
	UFUNCTION()
	void SwallowPlayer(AUnderwaterCharacter* Victim);
	UFUNCTION()
	void EjectPlayer(AUnderwaterCharacter* Victim);

	virtual void NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor) override;

	virtual void OnDeath() override;

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

	UFUNCTION()
	void TemporarilyDisalbeSightPerception(float Duration);

	UFUNCTION()
	void SightPerceptionOn();

	UFUNCTION()
	void SetPatrolStateAfterEject();

	UFUNCTION()
	void ApplyFleeAfterSwallow();
	void ClearSwallowTimer();

	UFUNCTION()
	void ForceEjectIfStuck();

	UFUNCTION()
	void DamageToVictim(AUnderwaterCharacter* Victim, float Damage);

	// void InitializeAggroVariable();
	void UpdateVictimLocation(float DeltaTime);
	void EjectedVictimNormalize(AUnderwaterCharacter* Victim);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> HorrorCreatureHitSphere;

	UPROPERTY()
	TObjectPtr<AUnderwaterCharacter> SwallowedPlayer;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> EjectMontage;

	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> CachedPerceptionComponent;

	FTimerHandle TimerHandle_SetSwimMode;
	FTimerHandle SwallowToFleeTimerHandle; // 삼키고 도망칠때 0.5초 delay (먹는모션, 소리 재생 )
	FTimerHandle ForceEjectTimerHandle; // 먹고나서 강제로 뱉게하기 위한 타이머핸들러

private:
	UPROPERTY(EditAnywhere, Category = "Lanch")
	float LanchStrength = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Sight")
	float DisableSightTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Behavior")
	float FleeTime;

	UPROPERTY(EditAnywhere, Category = "Swallow")
	float SwallowDamage;

	UPROPERTY(EditAnywhere, Category = "Shallow")
	float SwallowSpeed = 1.5f;

	// 강제로 뱉게 하기 위한 시간 리미트 (6.0초)
	UPROPERTY(EditAnywhere, Category = "Shallow")
	float ForceEjectAfterSeconds = 6.0f;

	// Variable about SwallowPlayer Location Lerp
	FVector VictimLocation;
	FVector CreatureMouthLocation;
	float SwallowLerpAlpha = 0.0f;
	uint8 bSwallowingInProgress : 1 = false;

	uint8 bCanSwallow : 1 = true;
#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return HorrorCreatureHitSphere; }
	AUnderwaterCharacter* GetSwallowedPlayer() { return SwallowedPlayer; }
	bool GetbCanSwallow() { return bCanSwallow; }

#pragma endregion
};
