// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "DataProviders/AIDataProvider.h"
#include "EQG_XYZGrid.generated.h"


UCLASS(EditInlineNew, Category = "Generators", meta = (DisplayName = "XYZ 3D Grid"))
class ABYSSDIVERUNDERWORLD_API UEQG_XYZGrid : public UEnvQueryGenerator
{
	GENERATED_BODY()
	
public:
	UEQG_XYZGrid();

	
protected:

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;


protected:

	bool IsLocationBlocked(UWorld* World, const FVector& Location, float TestSphereRadius,
		const FCollisionQueryParams& QueryParams) const;

	bool CanMoveBetweenPoints(UWorld* World, const FVector& From, const FVector& To,
		const FCollisionQueryParams& QueryParams) const;

	void AddNeighborToQueue(const FIntVector& NeighborGrid, const FVector& NeighborLocation,
		const FVector& CurrentLocation, UWorld* World,
		const FCollisionQueryParams& QueryParams,
		TSet<FIntVector>& OutVisitedCells, TQueue<FIntVector>& OutCellQueue) const;

#pragma region Variables
	// Location creation criteria
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> GenerateAround;

	// X/Y/Z Setting the scope
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue GridHalfSizeXY;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue GridHalfHeightZ;

	// Point spacing
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue PointSpacing;

	// 아이템을 특정 자리에 생성할지 말지 결정 할 때 장애물 체크 반경
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	float ObstacleCheckRadius = 30.0f;

#pragma endregion
};

