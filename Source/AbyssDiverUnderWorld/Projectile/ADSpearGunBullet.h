﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ADProjectileBase.h"
#include "ADSpearGunBullet.generated.h"

enum class ESFX : uint8;
class UDataTableSubsystem;
class USoundSubsystem;

UENUM(BlueprintType)
enum class ESpearGunType : uint8
{
	Basic = 0,
	Poison = 1,
	Bomb = 2,
	MAX UMETA(Hidden)
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADSpearGunBullet : public AADProjectileBase
{
	GENERATED_BODY()

	AADSpearGunBullet();

#pragma region Method
public:
	virtual void Deactivate() override;
protected:
	virtual void BeginPlay() override;
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(NetMulticast, Reliable)
	void M_SpawnFX(UNiagaraSystem* Effect, ESFX SFXType, const FVector& SpawnLocation);
	void M_SpawnFX_Implementation(UNiagaraSystem* Effect, ESFX SFXType, const FVector& SpawnLocation);

	UFUNCTION(NetMulticast, Reliable)
	void M_AdjustTransform(FTransform WorldTransform);
	void M_AdjustTransform_Implementation(FTransform WorldTransform);

	UFUNCTION()
	void OnRep_BulletType();

private:
	void AttachToHitActor(USceneComponent* HitComp, const FHitResult& Hit, bool bAttachOnHit);

	void ApplyAdditionalDamage();

	void Burst();

	void Addict();
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UDataTableSubsystem> DataTableSubsystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<USoundSubsystem> SoundSubsystem;


private:
	UPROPERTY(ReplicatedUsing = OnRep_BulletType, VisibleAnywhere, Category = "Bullet")
	ESpearGunType BulletType;
	UPROPERTY(VisibleAnywhere, Category = "Bullet")
	int16 AdditionalDamage;
	UPROPERTY(VisibleAnywhere, Category = "Bullet")
	int8 PoisonDuration;

	uint8 bWasAdditionalDamage : 1;

	FRotator InitialRotator = FRotator::ZeroRotator;

#pragma endregion

#pragma region Getter/Setter
public:
	//프로젝타일 스폰 후 지정
	UFUNCTION(BlueprintCallable)
	void SetBulletType(ESpearGunType NewBulletType) { BulletType = NewBulletType; }; 

	USoundSubsystem* GetSoundSubsystem();

#pragma endregion
	
};
