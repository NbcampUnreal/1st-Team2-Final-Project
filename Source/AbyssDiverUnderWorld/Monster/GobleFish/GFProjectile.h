// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GFProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AGFProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AGFProjectile();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	void FireDirection(const FVector& ShootDirection);
	void DestroyProjectile();

private:
	UFUNCTION()
	void OnProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

#pragma endregion

#pragma region Variable
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;
	
	// Niagara Particle
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> TrailEffect;
	// Niagara Particle Instance. ( Spawned on creation )
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float Damage;
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan;
#pragma endregion
};
