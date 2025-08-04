#include "Interactable/Item/Weapon/ADMine.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Subsystems/SoundSubsystem.h"
#include "Framework/ADGameInstance.h"

AADMine::AADMine()
{
	bReplicates = true;
	SetReplicateMovement(true);

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->InitSphereRadius(20.f);  // 실제 충돌용(작게)
	SphereComp->SetCollisionProfileName("OverlapAll");
	RootComponent = SphereComp;
}


void AADMine::BeginPlay()
{
	Super::BeginPlay();
	SphereComp->SetSphereRadius(20.f);
}

void AADMine::Explode()
{
	if (!HasAuthority()) 
		return;

	// 데미지 적용
	TArray<AActor*> Ignore;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		BaseDamage,
		10.f,                 
		GetActorLocation(),
		InnerRadius,
		OuterRadius,
		DamageFalloff,
		nullptr,              
		Ignore,
		this);

	// FX 적용
	if (ExplosionNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ExplosionNiagara, GetActorLocation());

	M_PlayExplodeSound();

	Destroy();
}

void AADMine::M_PlayExplodeSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::ExplodeMine, GetActorLocation());
}

USoundSubsystem* AADMine::GetSoundSubsystem()
{
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
}

