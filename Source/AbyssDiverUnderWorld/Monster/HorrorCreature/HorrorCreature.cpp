// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreature.h"
#include "GameFramework/CharacterMovementComponent.h"

AHorrorCreature::AHorrorCreature()
{
	// Initialize Variable
	ChaseTriggerTime = 2.0f;
	ChaseSpeed = 650.0f;
	PatrolSpeed = 150.0f;
	InvestigateSpeed = 250.0f;

	HorrorCreatureHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HorrorCreatureHitSphere"));
	HorrorCreatureHitSphere->SetupAttachment(GetMesh(), TEXT("AttackSocket"));
	HorrorCreatureHitSphere->InitSphereRadius(20.0f);
	HorrorCreatureHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HorrorCreatureHitSphere->SetHiddenInGame(true);
}

