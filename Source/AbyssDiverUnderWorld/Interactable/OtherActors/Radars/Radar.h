#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Radar.generated.h"

class URadarReturnComponent;

enum class EFriendOrFoe : uint8
{
	Friendly,
	Hostile,
	Neutral
};

UENUM()
enum class EGridRotationOption : uint8
{
	GridRotatesOnAllAxis,
	GridRotatesOnZYAxis,
	NoGridRotation,
	MaintainLocalRotation,
	ZRotationOnly,
	ZMatchesWorld
};

USTRUCT(BlueprintType)
struct FRadarOuterDimensionsMetersFromBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeX = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeX = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeY = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeY = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeZ = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeZ = 50.0f;
};

USTRUCT(BlueprintType)
struct FRadarCullingDimensionsMetersFromBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeZ = 0.0f;
};

USTRUCT(BlueprintType)
struct FRadarOuterDimensionsLimitMetersFromBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeX = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeX = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeY = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeY = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositiveRangeZ = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NegativeRangeZ = 50.0f;
};

UCLASS(Blueprintable)
class ABYSSDIVERUNDERWORLD_API ARadar : public AActor
{
	GENERATED_BODY()

public:
	ARadar();

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#pragma region Methods
public:

	void UpdateRadarSourceComponent(USceneComponent* NewRadarSourceLocation, USceneComponent* NewRadarSourceRotation);
	void AddReturn(URadarReturnComponent* RadarReturn, EFriendOrFoe FriendOrFoe);
	void RemoveReturn(URadarReturnComponent* RadarReturn, int32 InIndex);
	void RemoveAllReturns();

	// 레이더에 표시되는 리턴들 크기 조절
	void ChangeReturnsSize(float NewSize);
	// 레이더 반경을 넓히면 그만큼 레이더 리턴들도 크기가 작아진다
	void ChangeRadarRadius(float NewRadius);

private:

	void FindIfReturnIsValidResponseForRader(AActor* RadarActor);
	void InitializeRadarReturnIfActive();
	void DetermineAndStoreAffiliation();
	void CheckIfUnitTypeIsVisible();
	void FindIfReturnsInVisibleRange();
	void AllowNonPrimaryReturnsWithoutRangeCheck();

	void UpdateOnlyVisibleToOwner();
	void ClearBadReturns();
	void UpdateGridVisibility();
	void RotateRadarGrid();
	void FindRadarReturnTransformFromGrid();

	void ConvertCurrentTransformToRelative();
	void FindTransformForPillarsOrPings();

	void UpdateExistingReturnMesh();
	void AddNewReturnMesh();

	void LimitOpacityToRadarDisplay(UStaticMeshComponent* RadarReturnMesh);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<USceneComponent> DefaultRadarRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<UStaticMeshComponent> DefaultRadarSourceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<USceneComponent> RadarSourceLocationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<USceneComponent> RadarSourceRotationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<UStaticMeshComponent> RadarGrid;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Radar")
	TObjectPtr<UStaticMeshComponent> GridCenterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	TArray<TObjectPtr<AActor>> IgnoreActorsArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	TArray<FName> FriendlyFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	TArray<FName> HostileFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	TMap<FName, float> FactionRangeMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	TMap<FName, float> UnitTypeRangeMulitpliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	uint8 bShouldShowFriendlies : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	uint8 bShouldShowHostiles : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Faction Filter")
	uint8 bShouldShowNonFactionAllignedReturns : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UStaticMesh> GridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UMaterialInterface> GridMat0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UMaterialInterface> GridMat1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UMaterialInterface> GridMat2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UMaterialInterface> GridMat3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	TObjectPtr<UMaterialInterface> GridMat4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Grid Customization")
	FVector GridMeshSizeMultiplier = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	FName RadarTagToFindActors = TEXT("Radar");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	uint8 bShouldUseSphere : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	float RadarOuterDimensionRadiusMetersFromSphere = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	float RadarOuterDimensionLimitRadiusMetersFromSphere = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	float RadarCullingRadiusMetersFromSphere = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	FRadarOuterDimensionsMetersFromBox RadarOuterDimensionsMetersFromBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	FRadarCullingDimensionsMetersFromBox RadarCullingDimensionsMetersFromBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	FRadarOuterDimensionsLimitMetersFromBox RadarOuterDimensionsLimitMetersFromBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Search Area Options")
	float RadarSearchAngle = 180;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bIsGridVisible : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bIsRadarActive : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	EGridRotationOption GridRotationOption = EGridRotationOption::GridRotatesOnAllAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bAreRadarReturnsReletiveToGrid : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bHaveReturnsStands : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bArePingsOnGrid : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	float ScaleReturnSizes = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	float FadeInDistanceMultiplier = 0.95;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bShouldAllowOpacityLimitedToDisplay : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar|Apperance Options")
	uint8 bIsOnlyVisibleToOwner : 1;

	UPROPERTY()
	TArray<TObjectPtr<URadarReturnComponent>> ReturnsArray;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> RadarReturnOwners;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ReturnsMeshes;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ReturnStandMeshes;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ReturnPingMeshes;

	TArray<EFriendOrFoe> FriendOrFoeStatusArray;

	EFriendOrFoe CurrentFOFStatus;
	TObjectPtr<URadarReturnComponent> CurrentRadarReturn;
	TObjectPtr<URadarReturnComponent> CurrentRadarReturnSearched;

	TObjectPtr<URadarReturnComponent> LastPrimaryReturn;

	TArray<FName> CurrentFactionTags;
	TArray<FName> CurrentUnitsTypeTags;

	uint8 bIsIgnoreRange : 1;
	uint8 bIsPrimaryValid : 1;
	uint8 bIsCurrentRadarPrimaryReturn : 1;
	uint8 bIsAllowingNonPrimaryReturnsWithoutRangeCheck : 1;
	uint8 bIsViableReturn : 1;

	uint8 bWasValidPrimaryReturnFound : 1;

	uint8 bIsVisibleToOwnerCached : 1;

	uint8 bIsStandHidden : 1;
	
	float CurrentRangeMultiplier = 0.0f;
	float CurrentRadarWidth = 1.0f;

	TArray<TObjectPtr<URadarReturnComponent>> CurrentRadarReturns;

	int32 CurrentIndexCached = 0;

	FTransform CurrentMeshTransform;
	FTransform CurrentStandTransform;

	int32 FoundReturnsCount = 0;

#pragma endregion

#pragma region Getters, Setters

public:

	const FName& GetRadarTag() const;
	void SetRadarTag(const FName& NewTag);

#pragma endregion

};
