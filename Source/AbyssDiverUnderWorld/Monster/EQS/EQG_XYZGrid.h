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

#pragma region Variable
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
};

