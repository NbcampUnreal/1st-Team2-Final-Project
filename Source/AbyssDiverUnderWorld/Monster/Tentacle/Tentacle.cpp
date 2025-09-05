// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Tentacle/Tentacle.h"

ATentacle::ATentacle()
{
	PrimaryActorTick.bCanEverTick = false;
	TentacleHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TentacleHitSphere"));
	TentacleHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	TentacleHitSphere->InitSphereRadius(20.0f);
	TentacleHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TentacleHitSphere->SetHiddenInGame(true);
}
