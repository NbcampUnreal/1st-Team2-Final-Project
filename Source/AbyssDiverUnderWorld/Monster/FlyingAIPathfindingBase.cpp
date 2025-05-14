// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/FlyingAIPathfindingBase.h"
#include "AbyssDiverUnderWorld.h"


UFlyingAIPathfindingBase::UFlyingAIPathfindingBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlyingAIPathfindingBase::BeginPlay()
{
	Super::BeginPlay();
}

void UFlyingAIPathfindingBase::S_MoveTo_Implementation(FVector TargetLocation)
{
	MoveTo(TargetLocation);
}

void UFlyingAIPathfindingBase::MoveTo_Implementation(FVector TargetLocation)
{
	LOG(TEXT("C++ MoveTo_Implementation called. It's BP fallback Method"));
}

void UFlyingAIPathfindingBase::TriggerMoveFinished()
{
	OnFinishedMoving.Broadcast();
	LOG(TEXT("Arrived, TriggerMoveFinished Node is Triggerd. Delegate Broadcasing"))
}



