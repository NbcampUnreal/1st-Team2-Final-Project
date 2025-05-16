// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Tentacle/Tentacle.h"

ATentacle::ATentacle()
{
	TentacleHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TentacleHitSphere"));
	TentacleHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	TentacleHitSphere->InitSphereRadius(20.0f);
	TentacleHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TentacleHitSphere->SetHiddenInGame(true);

	UE_LOG(LogTemp, Warning, TEXT("Parent: %s, Socket: %s"),
		TentacleHitSphere->GetAttachParent() ? *TentacleHitSphere->GetAttachParent()->GetName() : TEXT("None"),
		*TentacleHitSphere->GetAttachSocketName().ToString());
}
