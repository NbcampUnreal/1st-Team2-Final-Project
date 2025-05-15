// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Tentacle/Tentacle.h"

ATentacle::ATentacle()
{
	TentacleHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TentacleHitSphere"));
	TentacleHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	TentacleHitSphere->InitSphereRadius(120.f);
	TentacleHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TentacleHitSphere->SetHiddenInGame(true);
}
