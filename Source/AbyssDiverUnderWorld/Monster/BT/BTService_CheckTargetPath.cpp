// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_CheckTargetPath.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monster/EMonsterState.h"
#include "NavigationSystem.h"
#include "Monster/Monster.h"

UBTService_CheckTargetPath::UBTService_CheckTargetPath()
{
	NodeName = TEXT("Check Target Path");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	Interval = 0.5f;
}

void UBTService_CheckTargetPath::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)  
{  
   Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);  

   UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();  
   if (!BB) return; 
   APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();  
   if (!AIPawn) return;  

   AMonster* Monster = Cast<AMonster>(AIPawn);

   AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
   if (!Target)  
   {
       if (Monster)
       {
           Monster->SetMonsterState(EMonsterState::Patrol);
       }
       BB->ClearValue(TargetActorKey.SelectedKeyName);  
       return;  
   }  

   // Get NavSys, NavData
   UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIPawn->GetWorld());  
   const ANavigationData* NavData = NavSys ? NavSys->GetNavDataForProps(AIPawn->GetNavAgentPropertiesRef(), AIPawn->GetActorLocation()) : nullptr;
   if (!NavSys || !NavData) return;

   FPathFindingQuery Query(  
       AIPawn->GetController(),  
       *NavSys->GetDefaultNavDataInstance(),  
       AIPawn->GetActorLocation(),  
       Target->GetActorLocation()  
   );  

   FPathFindingResult Result = NavSys->FindPathSync(Query);  
   if (Result.Result != ENavigationQueryResult::Success)  
   {  
       if (Monster)
       {
           Monster->SetMonsterState(EMonsterState::Patrol);
       }
       BB->ClearValue(TargetActorKey.SelectedKeyName);  
   }  
}
