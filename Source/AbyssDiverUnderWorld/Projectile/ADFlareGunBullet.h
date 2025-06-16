#pragma once

#include "CoreMinimal.h"
#include "Projectile/ADProjectileBase.h"
#include "ADFlareGunBullet.generated.h"

class UPointLightComponent;

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
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> FlareLight;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	float FlareIntensity = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	float FlareRadius = 3500.f;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	bool bUseInverseFalloff = false;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	FLinearColor FlareColor = FLinearColor(1.0f, 0.85f, 0.60f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	bool bCastShadows = false;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	bool bAffectTranslucent = true;

private:
	FTimerHandle HitDestroyHandle;
	float HitLifetime = 2.f;

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

};
