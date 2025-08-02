#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"

#include "Interactable/OtherActors/Radars/Radar2DComponent.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"

URadarReturn2DComponent::URadarReturn2DComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void URadarReturn2DComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAlwaysIgnore)
	{
		return;
	}
	
	// 등록하기
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (IsValid(PC) == false || PC->IsPendingKillPending() || PC->IsValidLowLevel() == false)
	{
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PC->GetPawn());
	if (IsValid(PlayerCharacter) == false || PlayerCharacter->IsPendingKillPending() || PlayerCharacter->IsValidLowLevel() == false)
	{
		return;
	}

	URadar2DComponent* RadarComp = PlayerCharacter->GetRadarComponent();
	if (IsValid(RadarComp) == false || RadarComp->IsBeingDestroyed() || RadarComp->IsValidLowLevel() == false)
	{
		return;
	}

	RadarComp->RegisterReturnComponent(this);
}

void URadarReturn2DComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 해제하기
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel())
	{
		Super::EndPlay(EndPlayReason);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (IsValid(PC) == false || PC->IsPendingKillPending() || PC->IsValidLowLevel() == false)
	{
		Super::EndPlay(EndPlayReason);
		return;
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PC->GetPawn());
	if (IsValid(PlayerCharacter) == false || PlayerCharacter->IsPendingKillPending() || PlayerCharacter->IsValidLowLevel() == false)
	{
		Super::EndPlay(EndPlayReason);
		return;
	}

	URadar2DComponent* RadarComp = PlayerCharacter->GetRadarComponent();
	if (IsValid(RadarComp) == false || RadarComp->IsBeingDestroyed() || RadarComp->IsValidLowLevel() == false)
	{
		Super::EndPlay(EndPlayReason);
		return;
	}

	RadarComp->UnregisterReturnComponent(this);
	
	Super::EndPlay(EndPlayReason);
}

float URadarReturn2DComponent::GetReturnScale() const
{
	return ReturnScale;
}

void URadarReturn2DComponent::SetReturnScale(float NewScale)
{
	ReturnScale = FMath::Max(0, NewScale);
}

EReturnForceType URadarReturn2DComponent::GetReturnForceType() const
{
	return ReturnForceType;
}

void URadarReturn2DComponent::SetReturnForceType(EReturnForceType NewReturnForceType)
{
	ReturnForceType = NewReturnForceType;
}

bool URadarReturn2DComponent::GetAlwaysDisplay() const
{
	return bAlwaysDisplay;
}

void URadarReturn2DComponent::SetAlwaysDisplay(bool bShouldAlwaysDisplay)
{
	bAlwaysDisplay = bShouldAlwaysDisplay;
}

bool URadarReturn2DComponent::GetAlwaysIgnore() const
{
	return bAlwaysIgnore;
}

void URadarReturn2DComponent::SetAlwaysIgnore(bool bShouldAlwaysIgnore)
{
	bAlwaysIgnore = bShouldAlwaysIgnore;
}

