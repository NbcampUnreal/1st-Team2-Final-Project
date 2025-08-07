// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/EQS/EQG_XYZGrid.h"
#include "AI/NavigationSystemBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Boss/Boss.h"
#include "Outsourced/AquaticMovementComponent.h"


UEQG_XYZGrid::UEQG_XYZGrid()  
{  
   ItemType = UEnvQueryItemType_Point::StaticClass();
   GenerateAround = UEnvQueryContext_Querier::StaticClass();
   GridHalfSizeXY.DefaultValue = 500.f;  
   GridHalfHeightZ.DefaultValue = 300.f;  
   PointSpacing.DefaultValue = 200.f;
}


void UEQG_XYZGrid::GenerateItems(FEnvQueryInstance& QueryInstance) const  
{  
   TArray<FVector> ContextLocations;  
   QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);

   const float HalfXY = GridHalfSizeXY.GetValue();  
   const float HalfZ = GridHalfHeightZ.GetValue();  
   float Spacing = PointSpacing.GetValue();  
   
   // Boss의 LOD 정보를 가져와서 스페이싱 조정
   if (ABoss* Boss = Cast<ABoss>(QueryInstance.Owner.Get()))
   {
       if (Boss->AquaticMovementComponent)
       {
           const float LODFactor = Boss->AquaticMovementComponent->CurrentLODFactor;
           // LOD가 0에 가까울수록(멀수록) 스페이싱을 늘림
           // LOD 1.0 = 기본 스페이싱, LOD 0.0 = 3배 스페이싱
           Spacing = Spacing * FMath::Lerp(3.0f, 1.0f, LODFactor);
       }
   }

   for (const FVector& Origin : ContextLocations)  
   {  
       for (float X = -HalfXY; X <= HalfXY; X += Spacing)  
       {  
           for (float Y = -HalfXY; Y <= HalfXY; Y += Spacing)  
           {  
               for (float Z = -HalfZ; Z <= HalfZ; Z += Spacing)  
               {  
                   const FVector Point = Origin + FVector(X, Y, Z);  
                   
                   // 수중 환경용: 장애물 내부가 아닌 모든 점을 유효한 것으로 처리
                   FCollisionQueryParams QueryParams;
                   QueryParams.AddIgnoredActor(Cast<AActor>(QueryInstance.Owner.Get()));
                   
                   // 위치가 장애물 내부가 아닌지 확인 (구체 오버랩)
                   const bool bIsInsideObstacle = QueryInstance.World->OverlapAnyTestByChannel(
                       Point,
                       FQuat::Identity,
                       ECC_Visibility,
                       FCollisionShape::MakeSphere(30.0f),  // 작은 구체로 체크
                       QueryParams
                   );
                   
                   // 장애물 내부가 아니면 유효한 지점
                   if (!bIsInsideObstacle)
                   {  
                       QueryInstance.AddItemData<UEnvQueryItemType_Point>(Point);
                   }  
               }  
           }  
       }  
   }  
}
