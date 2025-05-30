// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/EQS/EQG_XYZGrid.h"
#include "AI/NavigationSystemBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"


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
   const float Spacing = PointSpacing.GetValue();  

   for (const FVector& Origin : ContextLocations)  
   {  
       for (float X = -HalfXY; X <= HalfXY; X += Spacing)  
       {  
           for (float Y = -HalfXY; Y <= HalfXY; Y += Spacing)  
           {  
               for (float Z = -HalfZ; Z <= HalfZ; Z += Spacing)  
               {  
                   const FVector Point = Origin + FVector(X, Y, Z);  
                   // Register only when on top of NavMesh
                   FNavLocation NavLoc;  
                   if (UNavigationSystemV1::GetCurrent(QueryInstance.World)->ProjectPointToNavigation(Point, NavLoc, FVector(50, 50, 50)))  
                   {  
                       QueryInstance.AddItemData<UEnvQueryItemType_Point>(NavLoc.Location);
                   }  
               }  
           }  
       }  
   }  
}
