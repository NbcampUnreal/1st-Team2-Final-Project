#pragma once

#include "CoreMinimal.h"
#include "Projectile/ADProjectileBase.h"
#include "ADFlareGunBullet.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADFlareGunBullet : public AADProjectileBase
{
	GENERATED_BODY()
	
public:
	AADFlareGunBullet();

#pragma region Method
public:
	virtual void InitializeSpeed(const FVector& Dir, uint32 Speed) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;

protected:
	

private:
#pragma endregion

#pragma region Variable
public:


protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> FlareMesh;

private:
	FTimerHandle HitDestroyHandle;
	float HitLifetime = 2.f;

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

};
