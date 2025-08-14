#include "Projectile/ADShotgunBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Monster.h"

AADShotgunBullet::AADShotgunBullet()
{
	// 기반 SphereComponent는 생성자에서 이미 만들어졌으므로 필요시 반경 축소
	CollisionComponent->SetSphereRadius(3.0f);

	Damage = 80.f;                             // 펠릿 1발 데미지
	ProjectileMovementComp->InitialSpeed = 2000.f;
	ProjectileMovementComp->MaxSpeed = 2000.f;
	ProjectileMovementComp->ProjectileGravityScale = 0.f;
}

void AADShotgunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (OtherActor->IsA<AUnderwaterCharacter>() && OtherActor == GetInstigator()) return;

	HandleHit(OtherActor, SweepResult);
}

void AADShotgunBullet::HandleHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || HitActor == this) return;

	FVector KnockDir = GetVelocity().GetSafeNormal();
	//KnockDir.Z = 0.f;
	KnockDir.Normalize();

	bool bShouldKnockback = false;
	if (Cast<ABoss>(HitActor) || Cast<AMonster>(HitActor))
	{
		bShouldKnockback = true;
	}

	if (bShouldKnockback)
	{
		if (ACharacter* Char = Cast<ACharacter>(HitActor))
		{
			Char->LaunchCharacter(KnockDir * KnockbackStrength, true, true);  // xy‑방향만 변경
		}
		/* 물리 시뮬레이션 중인 다른 액터라면 Impulse */
		else if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Hit.GetComponent()))
		{
			if (Prim->IsSimulatingPhysics())
			{
				Prim->AddImpulseAtLocation(KnockDir * KnockbackStrength, Hit.ImpactPoint);
			}
		}
	}

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		Damage,
		GetActorForwardVector(),
		Hit,
		GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass());

	
	Deactivate();   // 베이스 클래스 Deactivate() 호출로 풀에 반환
}
