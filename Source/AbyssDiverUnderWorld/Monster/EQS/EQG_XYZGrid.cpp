// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/EQS/EQG_XYZGrid.h"
#include "AI/NavigationSystemBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
//#include "NavigationSystem.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Monster/Monster.h"
#include "Monster/Components/AquaticMovementComponent.h"


UEQG_XYZGrid::UEQG_XYZGrid()  
{  
   ItemType = UEnvQueryItemType_Point::StaticClass();
   GenerateAround = UEnvQueryContext_Querier::StaticClass();
   GridHalfSizeXY.DefaultValue = 500.f;  
   GridHalfHeightZ.DefaultValue = 300.f;  
   PointSpacing.DefaultValue = 200.f;
}


//void UEQG_XYZGrid::GenerateItems(FEnvQueryInstance& QueryInstance) const  
//{  
//   TArray<FVector> ContextLocations;  
//   QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);
//
//   const float HalfXY = GridHalfSizeXY.GetValue();  
//   const float HalfZ = GridHalfHeightZ.GetValue();  
//   float Spacing = PointSpacing.GetValue();  
//   
//   // Boss의 LOD 정보를 가져와서 스페이싱 조정
//   if (AMonster* Monster = Cast<AMonster>(QueryInstance.Owner.Get()))
//   {
//       if (Monster->AquaticMovementComponent)
//       {
//           const float LODFactor = Monster->AquaticMovementComponent->CurrentLODFactor;
//           // LOD가 0에 가까울수록(멀수록) 스페이싱을 늘림
//           // LOD 1.0 = 기본 스페이싱, LOD 0.0 = 3배 스페이싱
//           Spacing = Spacing * FMath::Lerp(3.0f, 1.0f, LODFactor);
//       }
//   }
//
//   for (const FVector& Origin : ContextLocations)  
//   {  
//       for (float X = -HalfXY; X <= HalfXY; X += Spacing)  
//       {  
//           for (float Y = -HalfXY; Y <= HalfXY; Y += Spacing)  
//           {  
//               for (float Z = -HalfZ; Z <= HalfZ; Z += Spacing)  
//               {  
//                   const FVector Point = Origin + FVector(X, Y, Z);  
//                   
//                   // 수중 환경용: 장애물 내부가 아닌 모든 점을 유효한 것으로 처리
//                   FCollisionQueryParams QueryParams;
//                   QueryParams.AddIgnoredActor(Cast<AActor>(QueryInstance.Owner.Get()));
//                   
//                   // 위치가 장애물 내부가 아닌지 확인 (구체 오버랩)
//                   const bool bIsInsideObstacle = QueryInstance.World->OverlapAnyTestByChannel(
//                       Point,
//                       FQuat::Identity,
//                       ECC_Visibility,
//                       FCollisionShape::MakeSphere(30.0f),  // 작은 구체로 체크
//                       QueryParams
//                   );
//                   
//                   // 장애물 내부가 아니면 유효한 지점
//                   if (!bIsInsideObstacle)
//                   {  
//                       QueryInstance.AddItemData<UEnvQueryItemType_Point>(Point);
//                   }  
//               }  
//           }  
//       }  
//   }  
//}

void UEQG_XYZGrid::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
    TArray<FVector> ContextLocations;
    QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);

    const float HalfXY = GridHalfSizeXY.GetValue();
    const float HalfZ = GridHalfHeightZ.GetValue();
    float Spacing = PointSpacing.GetValue();

    // Boss의 LOD 정보를 가져와서 스페이싱 조정
    //if (AMonster* Monster = Cast<AMonster>(QueryInstance.Owner.Get()))
    //{
    //    if (Monster->AquaticMovementComponent)
    //    {
    //        const float LODFactor = Monster->AquaticMovementComponent->CurrentLODFactor;
    //        Spacing = Spacing * FMath::Lerp(3.0f, 1.0f, LODFactor);
    //    }
    //}

    // Collision 파라미터 설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Cast<AActor>(QueryInstance.Owner.Get()));

    // 6방향 이웃 (상하좌우전후)
    static const TArray<FIntVector> Directions = 
    {
        FIntVector(1, 0, 0),   // +X
        FIntVector(-1, 0, 0),  // -X
        FIntVector(0, 1, 0),   // +Y
        FIntVector(0, -1, 0),  // -Y
        FIntVector(0, 0, 1),   // +Z
        FIntVector(0, 0, -1)   // -Z
    };

    for (const FVector& Origin : ContextLocations)
    {
        // Origin이 장애물 내부인 경우 스킵
        if (IsLocationBlocked(QueryInstance.World, Origin, ObstacleCheckRadius, QueryParams))
        {
            continue;
        }

        // Flood Fill을 위한 자료구조
        TSet<FIntVector> VisitedCells;
        TQueue<FIntVector> CellQueue;

        // 그리드 범위 계산
        const int32 MaxGridX = FMath::RoundToInt(HalfXY / Spacing);
        const int32 MaxGridY = FMath::RoundToInt(HalfXY / Spacing);
        const int32 MaxGridZ = FMath::RoundToInt(HalfZ / Spacing);

        // 시작점 (Origin = 0,0,0 in grid)
        FIntVector StartGrid(0, 0, 0);
        CellQueue.Enqueue(StartGrid);
        VisitedCells.Add(StartGrid);

        // BFS Flood Fill 실행
        int32 ProcessedCount = 0;
        const int32 MaxProcessCount = (2 * MaxGridX + 1) * (2 * MaxGridY + 1) * (2 * MaxGridZ + 1);

        while (!CellQueue.IsEmpty() && ProcessedCount < MaxProcessCount)
        {
            FIntVector CurrentGrid;
            CellQueue.Dequeue(CurrentGrid);
            ProcessedCount++;

            const FVector CurrentLocation = Origin + FVector(CurrentGrid.X * Spacing, CurrentGrid.Y * Spacing, CurrentGrid.Z * Spacing);

            // 6방향 기본 이웃 체크
            for (const FIntVector& Dir : Directions)
            {
                const FIntVector NeighborGrid = CurrentGrid + Dir;

                // 그리드 경계 체크
                if (FMath::Abs(NeighborGrid.X) > MaxGridX ||
                    FMath::Abs(NeighborGrid.Y) > MaxGridY ||
                    FMath::Abs(NeighborGrid.Z) > MaxGridZ)
                {
                    continue;
                }

                // 이미 방문한 셀은 스킵
                if (VisitedCells.Contains(NeighborGrid))
                {
                    continue;
                }

                const FVector NeighborLocation = Origin + FVector(NeighborGrid.X * Spacing, NeighborGrid.Y * Spacing, NeighborGrid.Z * Spacing);

                // 이웃 셀로 이동 가능한지 체크하고 큐에 추가
                AddNeighborToQueue(NeighborGrid, NeighborLocation, CurrentLocation, QueryInstance.World, QueryParams, VisitedCells, CellQueue);
            }
        }

        // 방문한 모든 셀을 QueryInstance에 추가
        for (const FIntVector& GridPos : VisitedCells)
        {
            const FVector WorldPos = Origin + FVector(GridPos.X * Spacing, GridPos.Y * Spacing, GridPos.Z * Spacing);

            QueryInstance.AddItemData<UEnvQueryItemType_Point>(WorldPos);
        }
    }
}

bool UEQG_XYZGrid::IsLocationBlocked(UWorld* World, const FVector& Location, float TestSphereRadius, const FCollisionQueryParams& QueryParams) const
{
    // 위치가 장애물 내부인지 확인
    return World->OverlapAnyTestByChannel(Location, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TestSphereRadius), QueryParams);
}

bool UEQG_XYZGrid::CanMoveBetweenPoints(UWorld* World, const FVector& From, const FVector& To, const FCollisionQueryParams& QueryParams) const
{
    // 두 점 사이의 직선 경로가 막혀있는지 체크
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(HitResult, From, To, ECC_Visibility, QueryParams);

    // 추가 안전 체크: 중간 지점도 확인
    if (!bHit)
    {
        FVector MidPoint = (From + To) * 0.5f;
        // 중간 지점이 장애물 내부가 아닌지 확인
        bHit = IsLocationBlocked(World, MidPoint, (MidPoint - From).Length(), QueryParams);
    }

    return !bHit;
}

void UEQG_XYZGrid::AddNeighborToQueue(const FIntVector& NeighborGrid, const FVector& NeighborLocation, const FVector& CurrentLocation,
    UWorld* World, const FCollisionQueryParams& QueryParams,
    TSet<FIntVector>& OutVisitedCells, TQueue<FIntVector>& OutCellQueue) const
{
    // 이웃 셀이 장애물 내부인지 체크
    if (IsLocationBlocked(World, NeighborLocation, ObstacleCheckRadius, QueryParams))
    {
        return;
    }

    // 현재 셀에서 이웃 셀로 이동 가능한지 체크
    if (!CanMoveBetweenPoints(World, CurrentLocation, NeighborLocation, QueryParams))
    {
        return;
    }

    // 유효한 셀이므로 방문 목록과 큐에 추가
    OutVisitedCells.Add(NeighborGrid);
    OutCellQueue.Enqueue(NeighborGrid);
}