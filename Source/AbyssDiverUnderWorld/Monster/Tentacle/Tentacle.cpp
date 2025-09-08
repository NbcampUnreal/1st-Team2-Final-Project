// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Tentacle/Tentacle.h"

ATentacle::ATentacle()
{
	PrimaryActorTick.bCanEverTick = true;
	TentacleHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TentacleHitSphere"));
	TentacleHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	TentacleHitSphere->InitSphereRadius(20.0f);
	TentacleHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TentacleHitSphere->SetHiddenInGame(true);
	TentacleHitSphere->ComponentTags.Add(TEXT("TentacleHitSphere"));
}

void ATentacle::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ATentacle::OnMeshOverlapBegin);
	TentacleHitSphere->OnComponentBeginOverlap.AddDynamic(this, &ATentacle::OnMeshOverlapBegin);
}
