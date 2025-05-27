#include "Gimmic/SafeZone/SafeZoneTriggerBox.h"
#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"

ASafeZoneTriggerBox::ASafeZoneTriggerBox()
{
}

void ASafeZoneTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &ASafeZoneTriggerBox::OnBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &ASafeZoneTriggerBox::OnEndOverlap);
}

void ASafeZoneTriggerBox::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	LOG(TEXT("Player In SeaWeed"));
	Player->SetHideInSeaweed(true);
	
}

void ASafeZoneTriggerBox::OnEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	LOG(TEXT("Player Out SeaWeed"));
	Player->SetHideInSeaweed(false);
}
