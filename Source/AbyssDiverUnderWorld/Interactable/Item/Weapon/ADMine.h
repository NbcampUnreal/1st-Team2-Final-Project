// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADMine.generated.h"

class USphereComponent;
class UNiagaraSystem;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADMine : public AActor
{
	GENERATED_BODY()
	
public:
	AADMine();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void Explode();

	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayExplodeSound();
	void M_PlayExplodeSound_Implementation();

	UFUNCTION(NetMulticast, Unreliable) 
	void Multicast_PlayExplodeFX();
	void Multicast_PlayExplodeFX_Implementation();

private:
	
#pragma endregion

#pragma region Variable
public:


protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComp;
	
	// FX
	UPROPERTY(EditDefaultsOnly) 
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

	// 폭발 파라미터 
	UPROPERTY(EditDefaultsOnly) 
	float BaseDamage = 600.f;
	UPROPERTY(EditDefaultsOnly) 
	float InnerRadius = 100.f;
	UPROPERTY(EditDefaultsOnly) 
	float OuterRadius = 250.f;
	UPROPERTY(EditDefaultsOnly) 
	float DamageFalloff = 1.0f;

#pragma endregion

#pragma region Getter, Setteer
public:

private:
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion
};
