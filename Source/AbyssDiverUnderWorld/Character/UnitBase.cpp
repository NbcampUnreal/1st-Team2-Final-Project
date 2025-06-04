// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitBase.h"

#include "StatComponent.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"

AUnitBase::AUnitBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	RadarReturnComponent = CreateDefaultSubobject<URadarReturnComponent>(TEXT("RardarReturn"));

	Tags.Add(FName("Radar"));
}

void AUnitBase::BeginPlay()
{
	Super::BeginPlay();
}

float AUnitBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	StatComponent->TakeDamage(Damage);
	
	return Damage;
}
