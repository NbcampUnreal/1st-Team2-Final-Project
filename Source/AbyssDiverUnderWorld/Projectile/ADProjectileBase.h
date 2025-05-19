// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AADProjectileBase();

protected:
	virtual void BeginPlay() override;

#pragma region Method
protected:
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	void InitializeStats();
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> CollisionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

	int16 Damage;

#pragma endregion

#pragma region Getter, Setter
public:
	UProjectileMovementComponent* GetProjectileMovementComp() const { return ProjectileMovementComp; }
	void SetDamage(int8 DamageAmount) { Damage = DamageAmount; };
	void SetProjectileSpeed(float Speed);

#pragma endregion
};
