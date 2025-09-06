// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/Commons/BTService_CheckLocationDistance.h"
#include "Container/BlackboardKeys.h"

#include "Monster/MonsterAIController.h"
#include "Monster/Monster.h"

#include "Character/UnderwaterCharacter.h"

UBTService_CheckLocationDistance::UBTService_CheckLocationDistance()
{
	NodeName = "CheckLocationDistance";
	bNotifyTick = true;
	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UBTService_CheckLocationDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AMonsterAIController* AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	AMonster* Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!AIController || !Monster)
	{
		UE_LOG(LogTemp, Error, TEXT("BTService_CheckLocationDistance : AIController, Monster Invalid"));
		return;
	}

	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB) return;
		
	FVector MonsterLocation = Monster->GetActorLocation();
	FVector TargetLocation = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);
	float Distance = FVector::Dist(MonsterLocation, TargetLocation);

	if (Distance <= AcceptRadius)
	{
		BB->SetValueAsBool(ReturnBoolKey.SelectedKeyName, true);
	}
	else
	{
		BB->SetValueAsBool(ReturnBoolKey.SelectedKeyName, false);
	}
}
