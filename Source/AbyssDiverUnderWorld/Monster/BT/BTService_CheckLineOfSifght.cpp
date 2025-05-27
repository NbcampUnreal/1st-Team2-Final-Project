// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_CheckLineOfSifght.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_CheckLineOfSifght::UBTService_CheckLineOfSifght()
{
	NodeName = TEXT("Check Line of Sight");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_CheckLineOfSifght::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target) return;

	bool bHasLOS = AIController->LineOfSightTo(Target);
	BB->SetValueAsBool(bInLineOfSightKey.SelectedKeyName, bHasLOS);
}
