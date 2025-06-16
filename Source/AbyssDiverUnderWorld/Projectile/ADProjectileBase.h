// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/PoolableItem.h"
#include "ADProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UDataTableSubsystem;

#define LOGP(Verbosity, Format, ...) UE_LOG(ProjectileLog, Verbosity, TEXT("[%s] %s(%s) %s"), LOG_NETMODEINFO, ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(ProjectileLog, Log, All);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADProjectileBase : public APoolableItem
{
	GENERATED_BODY()
	
public:	
	AADProjectileBase();

#pragma region Method
public:
	UFUNCTION(NetMulticast, Reliable)
	void M_EffectActivate(bool bActivate);
	void M_EffectActivate_Implementation(bool bActivate);

	void Activate() override;
	void Deactivate() override;

	void InitializeTransform(const FVector& Location, const FRotator& Rotation);
	virtual void InitializeSpeed(const FVector& ShootDirection, const uint32 Speed);

protected:
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> CollisionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTableSubsystem> DTSubsystem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<UNiagaraComponent> TrailEffect;

	int16 Damage;
	uint8 bWasHit : 1;
private:
	FTimerHandle LifeTimerHandle;
	FTimerHandle TrailDeactivateTimerHandle;
#pragma endregion

#pragma region Getter, Setter
public:
	UProjectileMovementComponent* GetProjectileMovementComp() const { return ProjectileMovementComp; }
	void SetDamage(int8 DamageAmount) { Damage = DamageAmount; };

#pragma endregion
};
