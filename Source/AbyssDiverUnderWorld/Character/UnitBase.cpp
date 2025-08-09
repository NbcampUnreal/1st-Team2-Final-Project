#include "UnitBase.h"

#include "StatComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"
#include "Subsystems/MissionSubsystem.h"

AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	RadarReturnComponent = CreateDefaultSubobject<URadarReturnComponent>(TEXT("RardarReturn"));
	RadarReturn2DComponent = CreateDefaultSubobject<URadarReturn2DComponent>(TEXT("RadarReturn2D"));

	Tags.Add(FName("Radar"));
}

void AUnitBase::BeginPlay()
{
	Super::BeginPlay();

	GetGameInstance()->GetSubsystem<UMissionSubsystem>()->RequestBinding(this);
}

void AUnitBase::Destroyed()
{
#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	GetGameInstance()->GetSubsystem<UMissionSubsystem>()->RequestUnbinding(this);

	Super::Destroyed();
}

float AUnitBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	StatComponent->TakeDamage(Damage);
	
	return Damage;
}
