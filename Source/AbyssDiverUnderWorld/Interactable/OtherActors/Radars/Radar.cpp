﻿#include "Interactable/OtherActors/Radars/Radar.h"

#include "RadarReturnComponent.h"
#include "AbyssDiverUnderWorld.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

const FName ARadar::MeshEmissiveParamName = TEXT("EmissiveMultiplier");
const FName ARadar::MeshEdgeEmissiveParamName = TEXT("EmissiveMultiplier_Edge");
const FName ARadar::ReturnEmissiveParamName = TEXT("ReturnEmissiveMultiplier");

ARadar::ARadar()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultRadarRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRadarSource"));
	SetRootComponent(DefaultRadarRoot);

	DefaultRadarSourceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultRadarSourceComponent"));
	DefaultRadarSourceComponent->SetupAttachment(RootComponent);

	GridCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridCenter"));
	GridCenterMesh->SetupAttachment(RootComponent);

	RadarGrid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadarGrid"));
	RadarGrid->SetupAttachment(GridCenterMesh);

	bIsRadarActive = true;
	bIsIgnoreRange = false;
	bIsPrimaryValid = false;
	bIsCurrentRadarPrimaryReturn = false;
	bIsAllowingNonPrimaryReturnsWithoutRangeCheck = false;
	bIsViableReturn = false;
	bShouldShowNonFactionAllignedReturns = true;
	bShouldShowHostiles = true;
	bShouldShowFriendlies = true;
	bShouldUseSphere = true;
	bWasValidPrimaryReturnFound = false;

	bIsOnlyVisibleToOwner = false;
	bIsVisibleToOwnerCached = false;

	bAreRadarReturnsReletiveToGrid = true;

	bIsGridVisible = true;

	bHaveReturnsStands = true;
	bArePingsOnGrid = true;
	bIsStandHidden = false;

	bShouldAllowOpacityLimitedToDisplay = true;
}

void ARadar::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (RadarGrid == nullptr)
	{
		return;
	}

	RadarGrid->SetStaticMesh(GridMesh);

	RadarGrid->SetMaterial(0, GridMat0);
	RadarGrid->SetMaterial(1, GridMat1);
	RadarGrid->SetMaterial(2, GridMat2);
	RadarGrid->SetMaterial(3, GridMat3);
	RadarGrid->SetMaterial(4, GridMat4);

	RadarGrid->SetRelativeScale3D(GridMeshSizeMultiplier);
	RadarGrid->SetVisibility(bIsGridVisible);
	RadarGrid->SetOnlyOwnerSee(bIsOnlyVisibleToOwner);
	bIsVisibleToOwnerCached = bIsOnlyVisibleToOwner;
}

void ARadar::BeginPlay()
{
	Super::BeginPlay();

	check(DefaultRadarSourceComponent);

	if (RadarSourceLocationComponent == nullptr)
	{
		RadarSourceLocationComponent = DefaultRadarSourceComponent;
	}

	if (RadarSourceRotationComponent == nullptr)
	{
		RadarSourceRotationComponent = DefaultRadarSourceComponent;
	}
}

void ARadar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(RadarSourceLocationComponent) == false || IsValid(RadarSourceRotationComponent) == false)
	{
		LOGVN(Error, TEXT("Radar Disabled"));

		bIsRadarActive = false;
		SetActorTickEnabled(false);

		Destroy(); // 캐릭터가 죽음 이후 부활할 때 이전 레이더는 여전히 남은 상태. 그래서 파괴 시킴
		return;
	}
	
	if (bIsRadarActive)
	{
		TArray<AActor*> ActorList;
		UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), RadarTagToFindActors, ActorList);

		for (const auto& FoundActor : ActorList)
		{
			if (IgnoreActorsArray.Contains(FoundActor))
			{
				if (RadarReturnOwners.Contains(FoundActor))
				{
					TArray<URadarReturnComponent*> RadarReturns;
					FoundActor->GetComponents<URadarReturnComponent>(RadarReturns);

					for (auto& RadarReturn : RadarReturns)
					{
						RemoveReturn(RadarReturn, 0);
					}
				}
			}
			else
			{
				TArray<URadarReturnComponent*> RadarReturns;
				FoundActor->GetComponents<URadarReturnComponent>(RadarReturns);

				bool bIsRemoved = false;
				for (auto& RadarReturn : RadarReturns)
				{
					if (RadarReturn->GetIgnore())
					{
						RemoveReturn(RadarReturn, 0);
						bIsRemoved = true;
					}
				}

				if (bIsRemoved == false)
				{
					FindIfReturnIsValidResponseForRader(FoundActor);
				}
			}
		}
	}
	else
	{
		RemoveAllReturns();
	}

	UpdateOnlyVisibleToOwner();
	ClearBadReturns();
	UpdateGridVisibility();
	RotateRadarGrid();
	FindRadarReturnTransformFromGrid();
}

void ARadar::UpdateRadarSourceComponent(USceneComponent* NewRadarSourceLocation, USceneComponent* NewRadarSourceRotation)
{
	RadarSourceLocationComponent = NewRadarSourceLocation;
	RadarSourceRotationComponent = NewRadarSourceRotation;

	if (RadarSourceLocationComponent == nullptr)
	{
		RadarSourceLocationComponent = RadarSourceRotationComponent;
	}

	if (RadarSourceRotationComponent == nullptr)
	{
		RadarSourceRotationComponent = RadarSourceLocationComponent;
	}
}

void ARadar::AddReturn(URadarReturnComponent* RadarReturn, EFriendOrFoe FriendOrFoe)
{
	int32 Index = ReturnsArray.Find(RadarReturn);

	if (Index != INDEX_NONE)
	{
		if (FriendOrFoeStatusArray.Num() <= Index)
		{
			FriendOrFoeStatusArray.SetNum(Index + 1);
		}

		FriendOrFoeStatusArray[Index] = FriendOrFoe;
		return;
	}

	ReturnsArray.Emplace(RadarReturn);
	Index = ReturnsArray.Num() - 1;

	if (RadarReturnOwners.Contains(GetOwner()) == false)
	{
		RadarReturnOwners.Emplace(GetOwner());
	}

	if (ReturnsMeshes.Num() <= Index)
	{
		ReturnsMeshes.SetNum(Index + 1);
	}

	if (ReturnStandMeshes.Num() <= Index)
	{
		ReturnStandMeshes.SetNum(Index + 1);
	}

	if (ReturnPingMeshes.Num() <= Index)
	{
		ReturnPingMeshes.SetNum(Index + 1);
	}

	if (FriendOrFoeStatusArray.Num() <= Index)
	{
		FriendOrFoeStatusArray.SetNum(Index + 1);
	}
}

void ARadar::RemoveReturn(URadarReturnComponent* RadarReturn, int32 InIndex)
{
	int32 Index;

	if (::IsValid(RadarReturn))
	{
		Index = ReturnsArray.Find(RadarReturn);
		if (Index == INDEX_NONE)
		{
			return;
		}
	}
	else
	{
		Index = InIndex;
	}

	if (ReturnsArray.IsValidIndex(Index))
	{
		ReturnsArray.RemoveAt(Index);
	}

	if (RadarReturnOwners.IsValidIndex(Index))
	{
		RadarReturnOwners.RemoveAt(Index);
	}

	if (ReturnsMeshes.IsValidIndex(Index))
	{
		UStaticMeshComponent* ReturnMesh = ReturnsMeshes[Index];

		if (::IsValid(ReturnMesh))
		{
			ReturnMesh->DestroyComponent();
		}

		ReturnsMeshes.RemoveAt(Index);
	}

	if (ReturnStandMeshes.IsValidIndex(Index))
	{
		UStaticMeshComponent* ReturnStandMesh = ReturnStandMeshes[Index];

		if (::IsValid(ReturnStandMesh))
		{
			ReturnStandMesh->DestroyComponent();
		}

		ReturnStandMeshes.RemoveAt(Index);
	}

	if (ReturnPingMeshes.IsValidIndex(Index))
	{
		UStaticMeshComponent* ReturnPingMesh = ReturnPingMeshes[Index];

		if (::IsValid(ReturnPingMesh))
		{
			ReturnPingMesh->DestroyComponent();
		}

		ReturnPingMeshes.RemoveAt(Index);
	}
}

void ARadar::RemoveAllReturns()
{
	int32 Count = ReturnsArray.Num();

	if (Count <= 0)
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		int RemovingIndex = Count - 1 - i;

		RemoveReturn(ReturnsArray[Count - 1 - i], RemovingIndex);
	}
}

void ARadar::ChangeReturnsSize(float NewSize)
{
	ScaleReturnSizes = NewSize;
}

void ARadar::ChangeRadarRadius(float NewRadius)
{
	RadarOuterDimensionRadiusMetersFromSphere = NewRadius;
	RadarOuterDimensionLimitRadiusMetersFromSphere = NewRadius;
}

void ARadar::ChangeRadarReturnMaterialEmissive(const float& NewEmissive)
{
	CurrentReturnEmissiveIntensity = NewEmissive;

	for (UStaticMeshComponent* ReturnMesh : ReturnsMeshes)
	{
		ReturnMesh->SetScalarParameterValueOnMaterials(ReturnEmissiveParamName, NewEmissive);
	}
}

void ARadar::ChangeRadarMeshMaterialEmissive(const float& NewEmissive)
{
	RadarGrid->SetScalarParameterValueOnMaterials(MeshEmissiveParamName, NewEmissive);
}

void ARadar::ChangeRadarMeshEdgeMaterialEmissive(const float& NewEmissive)
{
	RadarGrid->SetScalarParameterValueOnMaterials(MeshEdgeEmissiveParamName, NewEmissive);
}

void ARadar::FindIfReturnIsValidResponseForRader(AActor* RadarActor)
{
	bIsIgnoreRange = false;

	RadarActor->GetComponents<URadarReturnComponent>(CurrentRadarReturns);

	if (CurrentRadarReturns.IsValidIndex(0) == false)
	{
		return;
	}

	bIsPrimaryValid = false;
	LastPrimaryReturn = nullptr;

	if (CurrentRadarReturns.Num() > 1)
	{
		for (const auto& RadarReturn : CurrentRadarReturns)
		{
			if (RadarReturn->IsPrimaryReturn() == false)
			{
				continue;
			}

			CurrentRadarReturnSearched = RadarReturn;
			bIsCurrentRadarPrimaryReturn = true;
			InitializeRadarReturnIfActive();

			if (bWasValidPrimaryReturnFound)
			{
				bWasValidPrimaryReturnFound = false;
				break;
			}
		}

		if (bIsPrimaryValid == false)
		{
			int32 RadarReturnCount = CurrentRadarReturns.Num();
			for (int32 i = 0; i < RadarReturnCount; ++i)
			{
				RemoveReturn(CurrentRadarReturns[RadarReturnCount - 1 - i], 0);
			}

			return;
		}
	}
	else
	{
		if (CurrentRadarReturns.IsValidIndex(0) == false)
		{
			return;
		}

		CurrentRadarReturnSearched = CurrentRadarReturns[0];
		bIsCurrentRadarPrimaryReturn = false;
		InitializeRadarReturnIfActive();
	}

}

void ARadar::InitializeRadarReturnIfActive()
{
	if (CurrentRadarReturnSearched == nullptr)
	{
		return;
	}

	if (CurrentRadarReturnSearched->IsActivated() == false)
	{
		RemoveReturn(CurrentRadarReturnSearched, 0);
		return;
	}

	CurrentFactionTags = CurrentRadarReturnSearched->GetFactionTags();
	CurrentUnitsTypeTags = CurrentRadarReturnSearched->GetUnitsTypeTags();

	bIsViableReturn = false;
	CurrentRangeMultiplier = 0.0f;

	//if target has no faction, check if it should be accepted
	if (CurrentFactionTags.IsEmpty() == false)
	{
		DetermineAndStoreAffiliation();
		return;
	}

	if (bShouldShowNonFactionAllignedReturns)
	{
		//CheckIfUnitTypeIsVisible();
		// Fixed.. -> 수정 전엔 가장 최근의 Faction을 반영하도록 되어있었는듯?
		// 암튼 태그가 없으면 바로 Neutral로 판별하도록 Determine 함수 호출하도록 변경
		DetermineAndStoreAffiliation();
	}
	else
	{
		RemoveReturn(CurrentRadarReturnSearched, 0);
	}
}

void ARadar::DetermineAndStoreAffiliation()
{
	if (bShouldShowHostiles)
	{
		for (const auto& HostileFaction : HostileFactions)
		{
			if (CurrentFactionTags.Contains(HostileFaction) == false)
			{
				continue;
			}

			if (FactionRangeMultipliers.Contains(HostileFaction) && FactionRangeMultipliers[HostileFaction] == 0.0f)
			{
				continue;
			}

			bIsViableReturn = true;
			break;
		}

		if (bIsViableReturn)
		{
			CurrentFOFStatus = EFriendOrFoe::Hostile;
			CheckIfUnitTypeIsVisible();
			return;
		}
	}

	if (bShouldShowFriendlies)
	{
		for (const auto& FriendlyFaction : FriendlyFactions)
		{
			if (CurrentFactionTags.Contains(FriendlyFaction) == false)
			{
				continue;
			}

			if (FactionRangeMultipliers.Contains(FriendlyFaction) && FactionRangeMultipliers[FriendlyFaction] == 0.0f)
			{
				continue;
			}

			bIsViableReturn = true;
			break;
		}

		if (bIsViableReturn)
		{
			CurrentFOFStatus = EFriendOrFoe::Friendly;
			CheckIfUnitTypeIsVisible();
			return;
		}
	}

	if (bShouldShowNonFactionAllignedReturns)
	{
		CurrentFOFStatus = EFriendOrFoe::Neutral;
		CheckIfUnitTypeIsVisible();
	}
	else
	{
		RemoveReturn(CurrentRadarReturnSearched, 0);
	}
}

void ARadar::CheckIfUnitTypeIsVisible()
{
	bIsViableReturn = true;

	for (auto& TempUnitTypeTag : CurrentUnitsTypeTags)
	{
		if (UnitTypeRangeMulitpliers.Contains(TempUnitTypeTag) == false)
		{
			continue;
		}

		if (UnitTypeRangeMulitpliers[TempUnitTypeTag] != 0.0f)
		{
			continue;
		}

		bIsViableReturn = false;
		break;
	}

	if (bIsViableReturn)
	{
		if (bIsIgnoreRange == false)
		{
			FindIfReturnsInVisibleRange();
		}
		else
		{
			AddReturn(CurrentRadarReturnSearched, CurrentFOFStatus);
		}
	}
	else
	{
		RemoveReturn(CurrentRadarReturnSearched, 0);
	}
}

void ARadar::FindIfReturnsInVisibleRange()
{
	if (CurrentRadarReturnSearched == nullptr)
	{
		return;
	}

	if (IsValid(RadarSourceLocationComponent) == false || RadarSourceLocationComponent->IsValidLowLevel() == false)
	{
		LOGVN(Error, TEXT("RadarSourceLocationComponent is not valid"));
		return;
	}

	if (IsValid(RadarSourceRotationComponent) == false || RadarSourceRotationComponent->IsValidLowLevel() == false)
	{
		LOGVN(Error, TEXT("RadarSourceRotationComponent is not valid"));
		return;
	}

	switch (CurrentFOFStatus)
	{
	case EFriendOrFoe::Friendly:
		CurrentRangeMultiplier = CurrentRadarReturnSearched->GetVisibleRangeMultiplier() * CurrentRadarReturnSearched->GetFriendlyVisibleRangeMultiplier();
		break;
	case EFriendOrFoe::Hostile:
		CurrentRangeMultiplier = CurrentRadarReturnSearched->GetVisibleRangeMultiplier() * CurrentRadarReturnSearched->GetHostileVisibleRangeMultiplier();
		break;
	case EFriendOrFoe::Neutral:
		CurrentRangeMultiplier = CurrentRadarReturnSearched->GetVisibleRangeMultiplier() * CurrentRadarReturnSearched->GetNeutralVisibleRangeMultiplier();
		break;
	default:
		return;
	}

	for (const auto& TempFactionTag : CurrentFactionTags)
	{
		if (FactionRangeMultipliers.Contains(TempFactionTag) == false)
		{
			continue;
		}

		CurrentRangeMultiplier *= FactionRangeMultipliers[TempFactionTag];
	}

	for (const auto& TempUnitTypeTag : CurrentUnitsTypeTags)
	{
		if (UnitTypeRangeMulitpliers.Contains(TempUnitTypeTag) == false)
		{
			continue;
		}

		CurrentRangeMultiplier *= UnitTypeRangeMulitpliers[TempUnitTypeTag];
	}

	// 높이는 고려 하지 않음
	FVector AppliesToComponentLocation = CurrentRadarReturnSearched->GetAppliesToComponent()->GetComponentLocation();
	AppliesToComponentLocation.Z = 0;

	// 높이는 고려 하지 않음
	FVector RadarSourceLocation = RadarSourceLocationComponent->GetComponentLocation();
	RadarSourceLocation.Z = 0;

	FVector VectorToRadarSourceLocation = AppliesToComponentLocation - RadarSourceLocation;

	float PercentMultiplier = 100.0f;
	if (bShouldUseSphere)
	{
		float VisibleRange = VectorToRadarSourceLocation.Length();

		float RadarOuterDimensionRadius = RadarOuterDimensionRadiusMetersFromSphere * PercentMultiplier * CurrentRangeMultiplier;
		float RadarCullingRadius = RadarCullingRadiusMetersFromSphere * PercentMultiplier;
		float RadarOuterDimensionLimitRadius = RadarOuterDimensionLimitRadiusMetersFromSphere * PercentMultiplier;

		if ((VisibleRange <= RadarOuterDimensionRadius &&
			VisibleRange >= RadarCullingRadius &&
			VisibleRange <= RadarOuterDimensionLimitRadius) == false)
		{
			RemoveReturn(CurrentRadarReturnSearched, 0);
			return;
		}
	}
	else
	{
		FVector RadarSourceLocationScale = RadarSourceLocationComponent->GetComponentScale();
		FRotator RadarSourceRotation = RadarSourceRotationComponent->GetComponentRotation();
		FTransform RadarReturnTransform = CurrentRadarReturnSearched->GetAppliesToComponent()->GetComponentTransform();

		FTransform NewTransform(RadarSourceRotation, RadarSourceLocation, RadarSourceLocationScale);
		NewTransform = UKismetMathLibrary::MakeRelativeTransform(RadarReturnTransform, NewTransform);

		FVector NewLocation = NewTransform.GetLocation();

		bool bIsRadarOuterXInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.X, -RadarOuterDimensionsMetersFromBox.NegativeRangeX * PercentMultiplier, RadarOuterDimensionsMetersFromBox.PositiveRangeX * PercentMultiplier);
		bool bIsRadarOuterYInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Y, -RadarOuterDimensionsMetersFromBox.NegativeRangeY * PercentMultiplier, RadarOuterDimensionsMetersFromBox.PositiveRangeY * PercentMultiplier);
		bool bIsRadarOuterZInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Z, -RadarOuterDimensionsMetersFromBox.NegativeRangeZ * PercentMultiplier, RadarOuterDimensionsMetersFromBox.PositiveRangeZ * PercentMultiplier);

		bool bIsRadarCullingXInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.X, -RadarCullingDimensionsMetersFromBox.NegativeRangeX * PercentMultiplier, RadarCullingDimensionsMetersFromBox.PositiveRangeX * PercentMultiplier);
		bool bIsRadarCullingYInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Y, -RadarCullingDimensionsMetersFromBox.NegativeRangeY * PercentMultiplier, RadarCullingDimensionsMetersFromBox.PositiveRangeY * PercentMultiplier);
		bool bIsRadarCullingZInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Z, -RadarCullingDimensionsMetersFromBox.NegativeRangeZ * PercentMultiplier, RadarCullingDimensionsMetersFromBox.PositiveRangeZ * PercentMultiplier);

		bool bIsRadarOuterLimitXInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.X, -RadarOuterDimensionsLimitMetersFromBox.NegativeRangeX * PercentMultiplier, RadarOuterDimensionsLimitMetersFromBox.PositiveRangeX * PercentMultiplier);
		bool bIsRadarOuterLimitYInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Y, -RadarOuterDimensionsLimitMetersFromBox.NegativeRangeY * PercentMultiplier, RadarOuterDimensionsLimitMetersFromBox.PositiveRangeY * PercentMultiplier);
		bool bIsRadarOuterLimitZInRange = UKismetMathLibrary::InRange_FloatFloat(NewLocation.Z, -RadarOuterDimensionsLimitMetersFromBox.NegativeRangeZ * PercentMultiplier, RadarOuterDimensionsLimitMetersFromBox.PositiveRangeZ * PercentMultiplier);

		bool bIsRadarOuterDimensionsInRange = (bIsRadarOuterXInRange && bIsRadarOuterYInRange && bIsRadarOuterZInRange);
		bool bIsRadarCullingDimensionsNotInRange = (bIsRadarCullingXInRange == false && bIsRadarCullingYInRange == false && bIsRadarCullingZInRange == false);
		bool bIsRadarOuterDimensionsLimitInRange = (bIsRadarOuterLimitXInRange && bIsRadarOuterLimitYInRange && bIsRadarOuterLimitZInRange);

		if ((bIsRadarOuterDimensionsInRange && bIsRadarCullingDimensionsNotInRange && bIsRadarOuterDimensionsLimitInRange) == false)
		{
			RemoveReturn(CurrentRadarReturnSearched, 0);
			return;
		}
	}

	FVector DirectionToRadarSourceLocation = VectorToRadarSourceLocation;
	DirectionToRadarSourceLocation.Normalize();

	FVector RadarLookDirection = RadarSourceRotationComponent->GetForwardVector();
	RadarLookDirection.Normalize();

	float AngleToTargetDegree = FVector::DotProduct(DirectionToRadarSourceLocation, RadarLookDirection);

	float HalfDegree = 180.0f;
	if (RadarSearchAngle < HalfDegree && AngleToTargetDegree > RadarSearchAngle)
	{
		RemoveReturn(CurrentRadarReturnSearched, 0);
		return;
	}

	AddReturn(CurrentRadarReturnSearched, CurrentFOFStatus);

	if (bIsCurrentRadarPrimaryReturn)
	{
		AllowNonPrimaryReturnsWithoutRangeCheck();
		return;
	}
}

void ARadar::AllowNonPrimaryReturnsWithoutRangeCheck()
{
	if (CurrentRadarReturnSearched == nullptr)
	{
		return;
	}

	LastPrimaryReturn = CurrentRadarReturnSearched;
	bIsPrimaryValid = true;

	for (const auto& RadarReturn : CurrentRadarReturns)
	{
		if (LastPrimaryReturn == RadarReturn)
		{
			continue;
		}

		CurrentRadarReturnSearched = RadarReturn;
		bIsCurrentRadarPrimaryReturn = false;
		bIsIgnoreRange = true;
		InitializeRadarReturnIfActive();
	}

	bWasValidPrimaryReturnFound = true;
}

void ARadar::UpdateOnlyVisibleToOwner()
{
	if (bIsOnlyVisibleToOwner == bIsVisibleToOwnerCached)
	{
		return;
	}

	if (RadarGrid == nullptr)
	{
		return;
	}

	bIsVisibleToOwnerCached = bIsOnlyVisibleToOwner;

	RadarGrid->SetOnlyOwnerSee(bIsOnlyVisibleToOwner);

	for (auto& ReturnMesh : ReturnsMeshes)
	{
		ReturnMesh->SetOnlyOwnerSee(bIsOnlyVisibleToOwner);
	}
}

void ARadar::ClearBadReturns()
{
	int32 ReturnsCount = ReturnsArray.Num();

	for (int32 i = 0; i < ReturnsCount; ++i)
	{
		int32 Index = ReturnsCount - 1 - i;
		URadarReturnComponent* ReturnComponent = ReturnsArray[Index];
		if (::IsValid(ReturnComponent))
		{
			continue;
		}

		RemoveReturn(ReturnComponent, Index);
	}
}

void ARadar::UpdateGridVisibility()
{
	if (::IsValid(RadarGrid) == false)
	{
		return;
	}

	if (RadarGrid->IsVisible() == bIsGridVisible)
	{
		return;
	}

	RadarGrid->SetVisibility(bIsGridVisible);
}

void ARadar::RotateRadarGrid()
{
	if (GridCenterMesh == nullptr)
	{
		return;
	}

	if (IsValid(RadarSourceRotationComponent) == false || RadarSourceRotationComponent->IsValidLowLevel() == false)
	{
		LOGVN(Error, TEXT("RadarSourceRotationComponent is not valid"));
		return;
	}

	bool bIsRelative = false;

	float NewRotationRoll = 0.0f;
	float NewRotationPitch = 0.0f;
	float NewRotationYaw = 0.0f;

	FTransform ATransform;
	FTransform ReletiveTransform;

	FRotator NewRotation = RadarSourceRotationComponent->GetComponentRotation();

	switch (GridRotationOption)
	{
	case EGridRotationOption::GridRotatesOnAllAxis:
		break;
	case EGridRotationOption::GridRotatesOnZYAxis:

		NewRotationYaw = RadarSourceRotationComponent->GetComponentRotation().Yaw;
		break;
	case EGridRotationOption::NoGridRotation:

		NewRotationRoll = NewRotation.Roll;
		NewRotationPitch = NewRotation.Pitch;
		NewRotationYaw = NewRotation.Yaw;
		break;
	case EGridRotationOption::MaintainLocalRotation:

		bIsRelative = true;
		break;
	case EGridRotationOption::ZRotationOnly:

		bIsRelative = true;

		ATransform = FTransform(RadarSourceRotationComponent->GetComponentRotation(), FVector(0, 0, 0), FVector(1, 1, 1));
		ReletiveTransform = FTransform(GetActorRotation(), FVector(0, 0, 0), FVector(1, 1, 1));

		NewRotationYaw = UKismetMathLibrary::MakeRelativeTransform(ATransform, ReletiveTransform).Rotator().Yaw;

		break;
	case EGridRotationOption::ZMatchesWorld:

		bIsRelative = true;

		ATransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
		ReletiveTransform = FTransform(GetActorRotation(), FVector(0, 0, 0), FVector(1, 1, 1));

		NewRotationYaw = UKismetMathLibrary::MakeRelativeTransform(ATransform, ReletiveTransform).Rotator().Yaw;

		break;
	case EGridRotationOption::GridRotatesOnZYAxisInverse:

		NewRotationYaw =  -RadarSourceRotationComponent->GetComponentRotation().Yaw;
		break;
	default:
		check(false);
		return;
	}

	if (bIsRelative)
	{
		GridCenterMesh->SetWorldRotation(FRotator(NewRotationPitch, NewRotationYaw, NewRotationRoll));
	}
	else
	{
		GridCenterMesh->SetRelativeRotation(FRotator(NewRotationPitch, NewRotationYaw, NewRotationRoll));
	}
}

void ARadar::FindRadarReturnTransformFromGrid()
{
	if (IsValid(RadarSourceLocationComponent) == false || RadarSourceLocationComponent->IsValidLowLevel() == false)
	{
		LOGVN(Error, TEXT("RadarSourceLocationComponent is not valid"));
		return;
	}

	if (bShouldUseSphere)
	{
		CurrentRadarWidth = RadarOuterDimensionRadiusMetersFromSphere * 2.0f;
	}
	else
	{
		float Highest = FMath::Max3(RadarOuterDimensionsMetersFromBox.NegativeRangeX, RadarOuterDimensionsMetersFromBox.NegativeRangeY, RadarOuterDimensionsMetersFromBox.NegativeRangeZ);
		Highest = FMath::Max3(Highest, RadarOuterDimensionsMetersFromBox.PositiveRangeX, RadarOuterDimensionsMetersFromBox.PositiveRangeY);
		Highest = FMath::Max(Highest, RadarOuterDimensionsMetersFromBox.PositiveRangeZ);

		CurrentRadarWidth = Highest * 2.0f;
	}

	FoundReturnsCount = ReturnsArray.Num();
	for (int32 i = 0; i < FoundReturnsCount; ++i)
	{
		URadarReturnComponent* Return = ReturnsArray[i];
		if (::IsValid(Return) == false)
		{
			continue;
		}

		CurrentIndexCached = i;
		CurrentRadarReturn = Return;

		check(FriendOrFoeStatusArray.IsValidIndex(CurrentIndexCached));
		EFriendOrFoe FoF = FriendOrFoeStatusArray[CurrentIndexCached];

		USceneComponent* AppliesToComponent;

		FVector NewLocation;
		FRotator NewRotation;
		FVector NewScale;

		if (Return->GetScaleFromComponent() == nullptr || ScaleReturnSizes == 1.0f)
		{
			AppliesToComponent = Return->GetAppliesToComponent();
			check(AppliesToComponent);

			NewLocation = AppliesToComponent->GetComponentLocation() - RadarSourceLocationComponent->GetComponentLocation();
			NewLocation /= CurrentRadarWidth;

			NewRotation = AppliesToComponent->GetComponentRotation();
			NewScale = AppliesToComponent->GetComponentScale() * ScaleReturnSizes;

			switch (FoF)
			{
			case EFriendOrFoe::Friendly:

				NewScale *= Return->GetFriendlyMeshScale();
				break;
			case EFriendOrFoe::Hostile:

				NewScale *= Return->GetHostileMeshScale();
				break;
			case EFriendOrFoe::Neutral:

				NewScale *= Return->GetNeutralMeshScale();
				break;
			default:
				check(false);
				break;
			}

			NewScale /= CurrentRadarWidth;
		}
		else
		{
			AppliesToComponent = CurrentRadarReturn->GetAppliesToComponent();
			check(AppliesToComponent);

			USceneComponent* ScaleFromComponent = CurrentRadarReturn->GetScaleFromComponent();
			check(ScaleFromComponent);

			NewLocation = AppliesToComponent->GetComponentLocation() - ScaleFromComponent->GetComponentLocation();
			NewLocation *= ScaleReturnSizes;
			NewLocation += ScaleFromComponent->GetComponentLocation() - RadarSourceLocationComponent->GetComponentLocation();
			NewLocation /= CurrentRadarWidth;

			NewRotation = AppliesToComponent->GetComponentRotation();

			NewScale = AppliesToComponent->GetComponentScale() * ScaleReturnSizes;

			switch (FoF)
			{
			case EFriendOrFoe::Friendly:

				NewScale *= CurrentRadarReturn->GetFriendlyMeshScale();
				break;
			case EFriendOrFoe::Hostile:

				NewScale *= CurrentRadarReturn->GetHostileMeshScale();
				break;
			case EFriendOrFoe::Neutral:

				NewScale *= CurrentRadarReturn->GetNeutralMeshScale();
				break;
			default:
				check(false);
				break;
			}

			NewScale /= CurrentRadarWidth;
		}
		NewLocation.Z = 0;
		CurrentMeshTransform = FTransform(NewRotation, NewLocation, NewScale);

		if (FoundReturnsCount == 0)
		{
			return;
		}

		if (bAreRadarReturnsReletiveToGrid == false)
		{
			ConvertCurrentTransformToRelative();
		}

		FindTransformForPillarsOrPings();

		if (ReturnsMeshes.IsValidIndex(CurrentIndexCached))
		{

			if (::IsValid(ReturnsMeshes[CurrentIndexCached]))
			{
				UpdateExistingReturnMesh();
				continue;
			}
		}

		AddNewReturnMesh();
	}
}

void ARadar::ConvertCurrentTransformToRelative()
{
	FTransform ATransform(CurrentMeshTransform.GetRotation(), CurrentMeshTransform.GetLocation(), FVector(1, 1, 1));
	FTransform ReletiveTransform(CurrentMeshTransform.GetRotation(), FVector::ZeroVector, FVector::OneVector);

	CurrentMeshTransform = UKismetMathLibrary::MakeRelativeTransform(ATransform, ReletiveTransform);
}

void ARadar::FindTransformForPillarsOrPings()
{
	check(FriendOrFoeStatusArray.IsValidIndex(CurrentIndexCached));
	check(CurrentRadarReturn);

	EFriendOrFoe FoF = FriendOrFoeStatusArray[CurrentIndexCached];

	float TempBorderDividedByPingScale = 0;

	switch (FoF)
	{
	case EFriendOrFoe::Friendly:

		TempBorderDividedByPingScale = 1.0f / CurrentRadarReturn->GetFriendlyMeshScale();
		break;
	case EFriendOrFoe::Hostile:

		TempBorderDividedByPingScale = 1.0f / CurrentRadarReturn->GetHostileMeshScale();
		break;
	case EFriendOrFoe::Neutral:

		TempBorderDividedByPingScale = 1.0f / CurrentRadarReturn->GetNeutralMeshScale();
		break;
	default:
		check(false);
		break;
	}

	if (bHaveReturnsStands == false && bArePingsOnGrid == false)
	{
		return;
	}

	FRotator NewRotator(0.0f, CurrentMeshTransform.Rotator().Yaw, 0.0f);
	FVector NewLocation(CurrentMeshTransform.GetLocation().X, CurrentMeshTransform.GetLocation().Y, 0.0f);

	float NewScaleX = CurrentMeshTransform.GetScale3D().X * CurrentRadarReturn->GetStandXYScaleMultiplier() * TempBorderDividedByPingScale;
	float NewScaleY = CurrentMeshTransform.GetScale3D().Y * CurrentRadarReturn->GetStandXYScaleMultiplier() * TempBorderDividedByPingScale;
	float NewScaleZ = CurrentMeshTransform.GetLocation().Z;

	if (CurrentMeshTransform.GetLocation().Z >= 0.0f)
	{
		NewScaleZ += CurrentMeshTransform.GetScale3D().Z * CurrentRadarReturn->GetStandZScaleOffset();
	}
	else
	{
		NewScaleZ -= CurrentMeshTransform.GetScale3D().Z * CurrentRadarReturn->GetStandZScaleOffset();
	}

	CurrentStandTransform = FTransform(NewRotator, NewLocation, FVector(NewScaleX, NewScaleY, NewScaleZ));

	if (NewScaleZ < 0)
	{
		bIsStandHidden = true;
	}
}

void ARadar::UpdateExistingReturnMesh()
{
	check(ReturnsMeshes.IsValidIndex(CurrentIndexCached));

	UStaticMeshComponent* ReturnMesh = ReturnsMeshes[CurrentIndexCached];

	FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);

	if (GridRotationOption == EGridRotationOption::NoGridRotation)
	{
		ReturnMesh->AttachToComponent(GridCenterMesh, Rules);
	}
	else
	{
		ReturnMesh->AttachToComponent(RadarGrid, Rules);
	}

	ReturnMesh->SetRelativeTransform(CurrentMeshTransform);

	check(FriendOrFoeStatusArray.IsValidIndex(CurrentIndexCached));

	UStaticMesh* ApplyingMesh = nullptr;
	UMaterialInterface* ApplyingMaterial = nullptr;

	EFriendOrFoe FoF = FriendOrFoeStatusArray[CurrentIndexCached];
	switch (FoF)
	{
	case EFriendOrFoe::Friendly:

		ApplyingMesh = CurrentRadarReturn->GetFriendlyMesh();
		ApplyingMaterial = CurrentRadarReturn->GetFriendlyMaterial();
		break;
	case EFriendOrFoe::Hostile:

		ApplyingMesh = CurrentRadarReturn->GetHostileMesh();
		ApplyingMaterial = CurrentRadarReturn->GetHostileMaterial();
		break;
	case EFriendOrFoe::Neutral:

		ApplyingMesh = CurrentRadarReturn->GetNeutralMesh();
		ApplyingMaterial = CurrentRadarReturn->GetNeutralMaterial();
		break;
	default:
		check(false);
		break;
	}

	if (IsValid(ApplyingMesh) == false || ApplyingMesh->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMesh is not valid"));
		return;
	}

	if (IsValid(ApplyingMaterial) == false || ApplyingMaterial->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMaterial is not valid"));
		return;
	}

	ReturnMesh->SetStaticMesh(ApplyingMesh);
	ReturnMesh->SetMaterial(0, ApplyingMaterial);

	ReturnMesh->SetScalarParameterValueOnMaterials(ReturnEmissiveParamName, CurrentReturnEmissiveIntensity);

	if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplay())
	{
		LimitOpacityToRadarDisplay(ReturnMesh);
	}

	if (ReturnStandMeshes.IsValidIndex(CurrentIndexCached) == false)
	{
		return;
	}

	UStaticMeshComponent* ReturnStandMesh = ReturnStandMeshes[CurrentIndexCached];

	if (CurrentRadarReturn->HasReturnStand() && bHaveReturnsStands && bIsStandHidden == false)
	{
		ReturnStandMesh->SetVisibility(true);

		if (GridRotationOption == EGridRotationOption::NoGridRotation)
		{
			ReturnStandMesh->AttachToComponent(GridCenterMesh, Rules);
		}
		else
		{
			ReturnStandMesh->AttachToComponent(RadarGrid, Rules);
		}

		ReturnStandMesh->SetRelativeTransform(CurrentStandTransform, false, nullptr, ETeleportType::TeleportPhysics);

		UStaticMesh* ApplyingStandMesh = nullptr;
		UMaterialInterface* ApplyingStandMaterial = nullptr;

		switch (FoF)
		{
		case EFriendOrFoe::Friendly:

			ApplyingStandMesh = CurrentRadarReturn->GetFriendlyStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetFriendlyStandMaterial();
			break;
		case EFriendOrFoe::Hostile:

			ApplyingStandMesh = CurrentRadarReturn->GetHostileStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetHostileStandMaterial();
			break;
		case EFriendOrFoe::Neutral:

			ApplyingStandMesh = CurrentRadarReturn->GetNeutralStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetNeutralStandMaterial();
			break;
		default:
			check(false);
			break;
		}

		if (IsValid(ApplyingStandMesh) == false || ApplyingStandMesh->IsValidLowLevel() == false)
		{
			LOGV(Error, TEXT("ApplyingMesh is not valid"));
			return;
		}

		if (IsValid(ApplyingStandMaterial) == false || ApplyingStandMaterial->IsValidLowLevel() == false)
		{
			LOGV(Error, TEXT("ApplyingMaterial is not valid"));
			return;
		}

		ReturnStandMesh->SetStaticMesh(ApplyingStandMesh);
		ReturnStandMesh->SetMaterial(0, ApplyingStandMaterial);
	}
	else
	{
		ReturnStandMesh->SetVisibility(false);
	}

	if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplayForStand())
	{
		LimitOpacityToRadarDisplay(ReturnStandMesh);
	}

	if (bArePingsOnGrid == false || CurrentRadarReturn->HasReturnPing() == false)
	{
		return;
	}

	if (ReturnPingMeshes.IsValidIndex(CurrentIndexCached) == false)
	{
		return;
	}

	UStaticMeshComponent* ReturnPingMesh = ReturnPingMeshes[CurrentIndexCached];

	if (GridRotationOption == EGridRotationOption::NoGridRotation)
	{
		ReturnPingMesh->AttachToComponent(GridCenterMesh, Rules);
	}
	else
	{
		ReturnPingMesh->AttachToComponent(RadarGrid, Rules);
	}

	FVector NewScale = CurrentStandTransform.GetScale3D() * CurrentRadarReturn->GetStandXYScaleMultiplier() * CurrentRadarReturn->GetPingScaleMultiplier();
	NewScale.Z = NewScale.Y;

	FTransform NewTransform(CurrentStandTransform.Rotator(), CurrentStandTransform.GetLocation(), NewScale);
	ReturnPingMesh->SetRelativeTransform(NewTransform);

	UStaticMesh* ApplyingPingMesh = nullptr;
	UMaterialInterface* ApplyingPingMaterial = nullptr;

	switch (FoF)
	{
	case EFriendOrFoe::Friendly:

		ApplyingPingMesh = CurrentRadarReturn->GetFriendlyPingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetFriendlyPingMaterial();
		break;
	case EFriendOrFoe::Hostile:

		ApplyingPingMesh = CurrentRadarReturn->GetHostilePingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetHostilePingMaterial();
		break;
	case EFriendOrFoe::Neutral:


		ApplyingPingMesh = CurrentRadarReturn->GetNeutralPingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetNeutralPingMaterial();
		break;
	default:
		check(false);
		break;
	}

	if (IsValid(ApplyingPingMesh) == false || ApplyingPingMesh->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMesh is not valid"));
		return;
	}

	if (IsValid(ApplyingPingMaterial) == false || ApplyingPingMaterial->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMaterial is not valid"));
		return;
	}

	ReturnPingMesh->SetStaticMesh(ApplyingPingMesh);
	ReturnPingMesh->SetMaterial(0, ApplyingPingMaterial);

	if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplayForPing())
	{
		LimitOpacityToRadarDisplay(ReturnPingMesh);
	}
}

void ARadar::AddNewReturnMesh()
{
	UStaticMeshComponent* TempMeshComponent = CastChecked<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));
	TempMeshComponent->SetOnlyOwnerSee(bIsOnlyVisibleToOwner);
	TempMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, true);
	if (GridRotationOption == EGridRotationOption::NoGridRotation)
	{
		TempMeshComponent->AttachToComponent(GridCenterMesh, Rules);
	}
	else
	{
		TempMeshComponent->AttachToComponent(RadarGrid, Rules);
	}

	TempMeshComponent->SetRelativeTransform(CurrentMeshTransform);

	check(FriendOrFoeStatusArray.IsValidIndex(CurrentIndexCached));

	if (IsValid(CurrentRadarReturn) == false || CurrentRadarReturn->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("CurrentRadarReturn Is Not Valid"));
		return;
	}

	UStaticMesh* ApplyingMesh = nullptr;
	UMaterialInterface* ApplyingMaterial = nullptr;

	EFriendOrFoe FoF = FriendOrFoeStatusArray[CurrentIndexCached];
	switch (FoF)
	{
	case EFriendOrFoe::Friendly:

		ApplyingMesh = CurrentRadarReturn->GetFriendlyMesh();
		ApplyingMaterial = CurrentRadarReturn->GetFriendlyMaterial();
		break;
	case EFriendOrFoe::Hostile:

		ApplyingMesh = CurrentRadarReturn->GetHostileMesh();
		ApplyingMaterial = CurrentRadarReturn->GetHostileMaterial();
		break;
	case EFriendOrFoe::Neutral:

		ApplyingMesh = CurrentRadarReturn->GetNeutralMesh();
		ApplyingMaterial = CurrentRadarReturn->GetNeutralMaterial();
		break;
	default:
		check(false);
		break;
	}

	if (IsValid(ApplyingMesh) == false || ApplyingMesh->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMesh is not valid"));
		return;
	}

	if (IsValid(ApplyingMaterial) == false || ApplyingMaterial->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMaterial is not valid"));
		return;
	}

	TempMeshComponent->SetStaticMesh(ApplyingMesh);
	TempMeshComponent->SetMaterial(0, ApplyingMaterial);
	
	TempMeshComponent->SetScalarParameterValueOnMaterials(ReturnEmissiveParamName, CurrentReturnEmissiveIntensity);
	check(ReturnsMeshes.IsValidIndex(CurrentIndexCached));
	ReturnsMeshes[CurrentIndexCached] = TempMeshComponent;

	if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplay())
	{
		LimitOpacityToRadarDisplay(ReturnsMeshes[CurrentIndexCached]);
	}

	if (bHaveReturnsStands && CurrentRadarReturn->HasReturnStand())
	{
		TempMeshComponent = CastChecked<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));
		TempMeshComponent->SetVisibility(bIsOnlyVisibleToOwner);
		TempMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TempMeshComponent->AttachToComponent(RadarGrid, Rules);
		TempMeshComponent->SetRelativeTransform(CurrentStandTransform);

		if (bIsStandHidden)
		{
			TempMeshComponent->SetVisibility(false);
		}

		UStaticMesh* ApplyingStandMesh = nullptr;
		UMaterialInterface* ApplyingStandMaterial = nullptr;

		switch (FoF)
		{
		case EFriendOrFoe::Friendly:

			ApplyingStandMesh = CurrentRadarReturn->GetFriendlyStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetFriendlyStandMaterial();
			break;
		case EFriendOrFoe::Hostile:

			ApplyingStandMesh = CurrentRadarReturn->GetHostileStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetHostileStandMaterial();
			break;
		case EFriendOrFoe::Neutral:

			ApplyingStandMesh = CurrentRadarReturn->GetNeutralStandMesh();
			ApplyingStandMaterial = CurrentRadarReturn->GetNeutralStandMaterial();
			break;
		default:
			check(false);
			break;
		}

		if (IsValid(ApplyingStandMesh) == false || ApplyingStandMesh->IsValidLowLevel() == false)
		{
			LOGV(Error, TEXT("ApplyingMesh is not valid"));
			return;
		}

		if (IsValid(ApplyingStandMaterial) == false || ApplyingStandMaterial->IsValidLowLevel() == false)
		{
			LOGV(Error, TEXT("ApplyingMaterial is not valid"));
			return;
		}

		TempMeshComponent->SetStaticMesh(ApplyingStandMesh);
		TempMeshComponent->SetMaterial(0, ApplyingStandMaterial);

		TempMeshComponent->SetScalarParameterValueOnMaterials(ReturnEmissiveParamName, CurrentReturnEmissiveIntensity);
		check(ReturnStandMeshes.IsValidIndex(CurrentIndexCached));
		ReturnStandMeshes[CurrentIndexCached] = TempMeshComponent;

		if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplayForStand())
		{
			LimitOpacityToRadarDisplay(ReturnStandMeshes[CurrentIndexCached]);
		}
	}

	if (CurrentRadarReturn->HasReturnPing() == false || bArePingsOnGrid == false)
	{
		return;
	}

	TempMeshComponent = CastChecked<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));
	TempMeshComponent->SetVisibility(bIsOnlyVisibleToOwner);
	TempMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TempMeshComponent->AttachToComponent(RadarGrid, Rules);

	FVector NewScale = CurrentStandTransform.GetScale3D() / CurrentRadarReturn->GetStandXYScaleMultiplier() * CurrentRadarReturn->GetPingScaleMultiplier();
	NewScale.Z = NewScale.Y;

	FTransform NewTransform(CurrentStandTransform.Rotator(), CurrentStandTransform.GetLocation(), NewScale);
	TempMeshComponent->SetRelativeTransform(NewTransform);

	UStaticMesh* ApplyingPingMesh = nullptr;
	UMaterialInterface* ApplyingPingMaterial = nullptr;

	switch (FoF)
	{
	case EFriendOrFoe::Friendly:

		ApplyingPingMesh = CurrentRadarReturn->GetFriendlyPingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetFriendlyPingMaterial();
		break;
	case EFriendOrFoe::Hostile:

		ApplyingPingMesh = CurrentRadarReturn->GetHostilePingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetHostilePingMaterial();
		break;
	case EFriendOrFoe::Neutral:


		ApplyingPingMesh = CurrentRadarReturn->GetNeutralPingMesh();
		ApplyingPingMaterial = CurrentRadarReturn->GetNeutralPingMaterial();
		break;
	default:
		check(false);
		break;
	}

	if (IsValid(ApplyingPingMesh) == false || ApplyingPingMesh->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMesh is not valid"));
		return;
	}

	if (IsValid(ApplyingPingMaterial) == false || ApplyingPingMaterial->IsValidLowLevel() == false)
	{
		LOGV(Error, TEXT("ApplyingMaterial is not valid"));
		return;
	}

	TempMeshComponent->SetStaticMesh(ApplyingPingMesh);
	TempMeshComponent->SetMaterial(0, ApplyingPingMaterial);

	TempMeshComponent->SetScalarParameterValueOnMaterials(ReturnEmissiveParamName, CurrentReturnEmissiveIntensity);
	check(ReturnPingMeshes.IsValidIndex(CurrentIndexCached));
	ReturnPingMeshes[CurrentIndexCached] = TempMeshComponent;

	if (CurrentRadarReturn->ShouldLimitOpacityToRadarDisplayForPing())
	{
		LimitOpacityToRadarDisplay(ReturnPingMeshes[CurrentIndexCached]);
	}
}

void ARadar::LimitOpacityToRadarDisplay(UStaticMeshComponent* RadarReturnMesh)
{
	if (bShouldAllowOpacityLimitedToDisplay == false)
	{
		return;
	}

	FVector ColorVector = GridCenterMesh->GetComponentLocation();
	UMaterialInstanceDynamic* MaterialInstance = RadarReturnMesh->CreateDynamicMaterialInstance(0, nullptr);

	MaterialInstance->SetVectorParameterValue(FName(TEXT("Location")), ColorVector);

	if (bShouldUseSphere)
	{
		MaterialInstance->SetScalarParameterValue(FName(TEXT("Use Sphere")), 1.0f);

		float MaterialParamValue = RadarGrid->GetComponentScale().X * 50.0f * FadeInDistanceMultiplier;
		MaterialInstance->SetScalarParameterValue(FName(TEXT("Radius")), MaterialParamValue);
	}
	else
	{
		MaterialInstance->SetScalarParameterValue(FName(TEXT("Use Sphere")), 0.0f);
	}
}

const FName& ARadar::GetRadarTag() const
{
	return RadarTagToFindActors;
}

void ARadar::SetRadarTag(const FName& NewTag)
{
	RadarTagToFindActors = NewTag;
}

void ARadar::SetGridRotationOption(EGridRotationOption Option)
{
	GridRotationOption = Option;
}