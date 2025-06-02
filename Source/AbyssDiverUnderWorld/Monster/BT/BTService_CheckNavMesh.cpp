// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_CheckNavMesh.h"
#include "AIController.h"
#include "BTService_CheckNavMesh.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckNavMesh::UBTService_CheckNavMesh()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Check NavMesh Status");
}

void UBTService_CheckNavMesh::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!Pawn) return;

    FVector Location = Pawn->GetActorLocation();
    FNavLocation Projected;

    bool bOnNav = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(
        Location, Projected, FVector(300, 300, 300));

    if (!bOnNav)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(ReturnToNavMeshLocationKey.SelectedKeyName, Projected.Location);
    }
    else
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(ReturnToNavMeshLocationKey.SelectedKeyName);
    }
}
