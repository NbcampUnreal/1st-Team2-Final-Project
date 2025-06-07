// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_MoveToNavmeshZone.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

UBTTask_MoveToNavmeshZone::UBTTask_MoveToNavmeshZone()
{
	NodeName = TEXT("Move To Safe NavMesh Location");
}

EBTNodeResult::Type UBTTask_MoveToNavmeshZone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;
    APawn* Pawn = AIController->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    FVector SafeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(ReturnToNavMeshLocationKey.SelectedKeyName);

    FNavLocation Projected;
    bool OnNav = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(
        SafeLocation, Projected, FVector(100, 100, 100));

    if (OnNav)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(Projected.Location);
        MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

        FNavPathSharedPtr NavPath;
        FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest, &NavPath);

        if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
        {
            return EBTNodeResult::Succeeded;
        }
    }

    // MoveTo Fail ¡æ Force Teleport
    Pawn->SetActorLocation(Projected.Location);
    return EBTNodeResult::Succeeded;
}
