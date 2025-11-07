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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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
	void ClearEjectTimer();

	UFUNCTION()
	void ForceEjectIfStuck();

	UFUNCTION()
	void DamageToVictim(AUnderwaterCharacter* Victim, float Damage);

	// void InitializeAggroVariable();
	void UpdateVictimLocation(float DeltaTime);
	void EjectedVictimNormalize(AUnderwaterCharacter* Victim);
	FVector CalculateSafeMouthLoc() const;

	void ClearAllTimers();

	void PlaySwallowFlipAnim(AUnderwaterCharacter* Victim);
	void StopSwallowFlipAnim(AUnderwaterCharacter* Victim);

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> HorrorCreatureHitSphere;

	UPROPERTY()
	TWeakObjectPtr<AUnderwaterCharacter> SwallowedPlayer;

	UPROPERTY()
	TWeakObjectPtr<UAIPerceptionComponent> CachedPerceptionComponent;

	FTimerHandle SetSwimModeTimerHandle;
	FTimerHandle SetPatrolTimerHandle;
	FTimerHandle EnableSightTimerHandle;
	FTimerHandle SwallowToFleeTimerHandle; // 삼키고 도망칠때 0.5초 delay (먹는모션, 소리 재생 )
	FTimerHandle ForceEjectTimerHandle; // 먹고나서 강제로 뱉게하기 위한 타이머핸들러

private:
	UPROPERTY(EditAnywhere, Category = "Launch")
	float LaunchStrength = 150.0f;

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
	
	// 플레이어가 완전히 입위치에 확인하는 플래그 변수
	uint8 bVictimLockedAtMouth : 1 = false;

	// 삼키는게 가능한지 확인하는 플래그 변수
	uint8 bCanSwallow : 1 = true;

	// Eject 중복 방지를 위한 플래그 변수
	uint8 bHasEjectedPlayer : 1 = false;
#pragma endregion

#pragma region Getter, Setter
public:
	virtual USphereComponent* GetAttackHitComponent() const override { return HorrorCreatureHitSphere; }
	AUnderwaterCharacter* GetSwallowedPlayer() { return SwallowedPlayer.Get(); }
	bool GetbCanSwallow() { return bCanSwallow; }

#pragma endregion
};
