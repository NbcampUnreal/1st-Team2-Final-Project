#include "Interactable/OtherActors/Radars/Radar2DComponent.h"

#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/Item/ADOreRock.h"

#include "EngineUtils.h"

URadar2DComponent::URadar2DComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void URadar2DComponent::BeginPlay()
{
	Super::BeginPlay();

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(GetOwner());
	if (PlayerCharacter == nullptr)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (PlayerCharacter->IsLocallyControlled() == false)
	{
		SetComponentTickEnabled(false);
		return;
	}

	
	RegisterCurrentReturns();
}

void URadar2DComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(GetOwner());
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	for (URadarReturn2DComponent* RadarReturn : CachedReturns)
	{
		if (IsValid(RadarReturn) == false || RadarReturn->IsBeingDestroyed() || RadarReturn->IsValidLowLevelFast() == false)
		{
			CachedReturnsInDetectRadius.Remove(RadarReturn);
			continue;
		}

		AActor* OtherActor = RadarReturn->GetOwner();
		if (IsValid(OtherActor) == false || OtherActor->IsPendingKillPending() || OtherActor->IsValidLowLevelFast() == false)
		{
			CachedReturnsInDetectRadius.Remove(RadarReturn);
			continue;
		}

		if (RadarReturn->GetAlwaysDisplay())
		{
			CachedReturnsInDetectRadius.Add(RadarReturn);
			continue;
		}

		float SquaredRadius = RadarDetectRadius * RadarDetectRadius;
		float SquaredDistance = FVector::DistSquaredXY(OtherActor->GetActorLocation(), PlayerCharacter->GetActorLocation());
		if (SquaredDistance <= SquaredRadius)
		{
			CachedReturnsInDetectRadius.Add(RadarReturn);
		}
		else
		{
			CachedReturnsInDetectRadius.Remove(RadarReturn);
		}
	}
}

void URadar2DComponent::RegisterReturnComponent(AActor* RegisteringActor)
{
	URadarReturn2DComponent* FoundRadarReturn = RegisteringActor->GetComponentByClass<URadarReturn2DComponent>();
	if (FoundRadarReturn == nullptr)
	{
		return;
	}

	RegisterReturnComponent(FoundRadarReturn);
}

void URadar2DComponent::RegisterReturnComponent(URadarReturn2DComponent* RegisteringReturn)
{
	CachedReturns.Add(RegisteringReturn);
}

void URadar2DComponent::UnregisterReturnComponent(AActor* RegisteredActor)
{
	URadarReturn2DComponent* FoundRadarReturn = RegisteredActor->GetComponentByClass<URadarReturn2DComponent>();
	if (FoundRadarReturn == nullptr)
	{
		return;
	}

	UnregisterReturnComponent(FoundRadarReturn);
}

void URadar2DComponent::UnregisterReturnComponent(URadarReturn2DComponent* RegisteredReturn)
{
	CachedReturns.Remove(RegisteredReturn);
	CachedReturnsInDetectRadius.Remove(RegisteredReturn);
}

void URadar2DComponent::RegisterCurrentReturns()
{
	UWorld* World = GetWorld();

	if (::IsValid(World) == false && World->IsInSeamlessTravel())
	{
		return;
	}

	for (AActor* RadarReturnOwner : TActorRange<AActor>(World))
	{
		URadarReturn2DComponent* FoundRadarReturn = RadarReturnOwner->GetComponentByClass<URadarReturn2DComponent>();
		if (FoundRadarReturn == nullptr)
		{
			continue;
		}

		if (HasReturnAlready(FoundRadarReturn))
		{
			continue;
		}

		RegisterReturnComponent(FoundRadarReturn);
	}
}

bool URadar2DComponent::HasReturnAlready(URadarReturn2DComponent* RadarReturn)
{
	return CachedReturns.Contains(RadarReturn);
}

bool URadar2DComponent::IsReturnInRadius(URadarReturn2DComponent* RadarReturn)
{
	return CachedReturnsInDetectRadius.Contains(RadarReturn);
}

const TSet<TObjectPtr<URadarReturn2DComponent>>& URadar2DComponent::GetAllReturnsInDetectRadius() const
{
	return CachedReturnsInDetectRadius;
}

float URadar2DComponent::GetRadarDetectRadius() const
{
	return RadarDetectRadius;
}
