// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ADProjectileBase.h"
#include "ADSpearGunBullet.generated.h"

class UDataTableSubsystem;

UENUM(BlueprintType)
enum class ESpearGunType : uint8
{
	Basic,
	Bomb,
	Poison
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADSpearGunBullet : public AADProjectileBase
{
	GENERATED_BODY()

	AADSpearGunBullet();

#pragma region Method
protected:
	virtual void BeginPlay() override;
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_BulletType();

private:
	void ApplyAdditionalDamage();

	void Burst();

	void Addict();
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UNiagaraComponent> TrailEffect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UDataTableSubsystem> DataTableSubsystem;
private:
	UPROPERTY(ReplicatedUsing = OnRep_BulletType, EditAnywhere, Category = "Bullet")
	ESpearGunType BulletType;
	UPROPERTY(VisibleAnywhere, Category = "Bullet")
	int8 AdditionalDamage;
	UPROPERTY(VisibleAnywhere, Category = "Bullet")
	int8 PoisonDuration;

	uint8 bWasHit : 1;

#pragma endregion

#pragma region Getter/Setter
private:
	//프로젝타일 스폰 후 지정
	UFUNCTION(BlueprintCallable)
	void SetBulletType(ESpearGunType NewBulletType) { BulletType = NewBulletType; }; 

#pragma endregion
	
};
