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

public:
	virtual void BeginPlay() override;

#pragma region Method

public:

	virtual void InitializeSpeed(const FVector& Dir, uint32 Speed) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void Activate() override;
	virtual void Deactivate() override;

protected:
	

private:
#pragma endregion

#pragma region Variable
public:


protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> FlareSphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> FlareMesh;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> FlareLight;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	float FlareIntensity = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	float FlareRadius = 3500.f;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	uint8 bUseInverseFalloff : 1 = false;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	FLinearColor FlareColor = FLinearColor(1.0f, 0.85f, 0.60f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	uint8 bCastShadows : 1 = false;

	UPROPERTY(EditAnywhere, Category = "Flare Light")
	uint8 bAffectTranslucent : 1 = true;
	UPROPERTY(EditAnywhere, Category = "Flare Light")
	float Lifetime = 5.f;

private:
	FTimerHandle HitDestroyHandle;
	float HitLifetime = 2.f;
	

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

};
