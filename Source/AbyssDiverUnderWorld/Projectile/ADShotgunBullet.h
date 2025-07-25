#pragma once

#include "CoreMinimal.h"
#include "Projectile/ADProjectileBase.h"
#include "ADShotgunBullet.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADShotgunBullet : public AADProjectileBase
{
	GENERATED_BODY()

public:
	AADShotgunBullet();

#pragma region Method

protected:

	/** ���� ���� �ǰ� ó�� */
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;

private:
	void HandleHit(AActor* HitActor, const FHitResult& Hit);

#pragma endregion
};
