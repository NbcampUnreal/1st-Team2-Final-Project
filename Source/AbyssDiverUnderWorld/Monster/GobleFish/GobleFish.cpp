// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/GobleFish/GobleFish.h"
#include "Monster/GobleFish/GFProjectile.h"

void AGobleFish::FireProjectile()
{
	if (!ProjectileClass) return;

	FVector FireLocation = GetMesh()->GetSocketLocation("ProjectileSocket");
	FRotator FireRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; // return AActor*
	SpawnParams.Instigator = GetInstigator(); // return APawn*
	
	GetWorld()->SpawnActor<AGFProjectile>(ProjectileClass, FireLocation, FireRotation, SpawnParams);
}

void AGobleFish::PlayAttackMontage()
{
}
