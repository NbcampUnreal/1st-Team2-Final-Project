#include "Projectile/ADFlareGunBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"

AADFlareGunBullet::AADFlareGunBullet()
{
	FlareMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlareMesh"));
	FlareMesh->SetupAttachment(RootComponent);
	FlareMesh->PrimaryComponentTick.bCanEverTick = false;

	FlareLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlareLight"));
	FlareLight->SetupAttachment(RootComponent);

	FlareSphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("FlareSphereCollision"));
	FlareSphereCollision->SetupAttachment(RootComponent);

	FlareLight->Intensity = FlareIntensity;   // 밝기
	FlareLight->AttenuationRadius = FlareRadius;    // 35 m 범위정도?
	FlareLight->bUseInverseSquaredFalloff = bUseInverseFalloff;
	FlareLight->SetLightColor(FlareColor);

	FlareLight->CastShadows = bCastShadows;     // **그림자 OFF**

	FlareLight->IntensityUnits = ELightUnits::Candelas;
}

void AADFlareGunBullet::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(HitDestroyHandle, this, &ThisClass::Deactivate, Lifetime, false);
}

void AADFlareGunBullet::InitializeSpeed(const FVector& Dir, uint32 Speed)
{
	Super::InitializeSpeed(Dir, Speed);
}

void AADFlareGunBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bWasHit) return;
	bWasHit = true;
	
	//ProjectileMovementComp->StopMovementImmediately();
	//AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform, SweepResult.BoneName);

	
}

void AADFlareGunBullet::Activate()
{
	Super::Activate();

	FlareMesh->PrimaryComponentTick.bCanEverTick = true;
	FlareMesh->SetComponentTickEnabled(true);
	FlareMesh->PrimaryComponentTick.bStartWithTickEnabled = true;
}

void AADFlareGunBullet::Deactivate()
{
	Super::Deactivate();

	FlareMesh->PrimaryComponentTick.bCanEverTick = false;
	FlareMesh->SetComponentTickEnabled(false);
	FlareMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
}
