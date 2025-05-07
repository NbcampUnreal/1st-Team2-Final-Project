#include "RadarReturnComponent.h"

URadarReturnComponent::URadarReturnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bShouldLimitOpacityToRadarDisplay = true;
	bHasReturnStand = true;
	bShouldLimitOpacityToRadarDisplayForStand = false;
	bHasReturnPing = true;
	bShouldLimitOpacityToRadarDisplayForPing = true;

	bIsPrimaryReturn = true;
	bIsActivated = true;
	
}

void URadarReturnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ApplyToComponentName == "")
	{
		AppliesToComponent = GetOwner()->GetRootComponent();
		return;
	}

	TArray<USceneComponent*> SceneCompoents;
	GetOwner()->GetComponents<USceneComponent>(SceneCompoents);

	for (auto& Component : SceneCompoents)
	{
		if (Component->GetName().Contains(ApplyToComponentName) == false)
		{
			continue;
		}

		AppliesToComponent = Component;
		break;
	}

	if (ScaleFromRootComponentName == "")
	{
		return;
	}

	for (auto& Component : SceneCompoents)
	{
		if (Component->GetName().Contains(ScaleFromRootComponentName) == false)
		{
			continue;
		}

		ScaleFromComponent = Component;
		break;
	}
}

void URadarReturnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

UStaticMesh* URadarReturnComponent::GetFriendlyMesh() const
{
	return FriendlyMesh;
}

UStaticMesh* URadarReturnComponent::GetHostileMesh() const
{
	return HostileMesh;
}

UStaticMesh* URadarReturnComponent::GetNeutralMesh() const
{
	return NeutralMesh;
}

UMaterialInterface* URadarReturnComponent::GetFriendlyMaterial() const
{
	return FriendlyMaterial;
}

UMaterialInterface* URadarReturnComponent::GetHostileMaterial() const
{
	return HostileMaterial;
}

UMaterialInterface* URadarReturnComponent::GetNeutralMaterial() const
{
	return NeutralMaterial;
}

UStaticMesh* URadarReturnComponent::GetFriendlyStandMesh() const
{
	return FriendlyStandMesh;
}

UStaticMesh* URadarReturnComponent::GetHostileStandMesh() const
{
	return HostileStandMesh;
}

UStaticMesh* URadarReturnComponent::GetNeutralStandMesh() const
{
	return NeutralStandMesh;
}

UMaterialInterface* URadarReturnComponent::GetFriendlyStandMaterial() const
{
	return FriendlyStandMaterial;
}

UMaterialInterface* URadarReturnComponent::GetHostileStandMaterial() const
{
	return HostileStandMaterial;
}

UMaterialInterface* URadarReturnComponent::GetNeutralStandMaterial() const
{
	return NeutralStandMaterial;
}

UStaticMesh* URadarReturnComponent::GetFriendlyPingMesh() const
{
	return FriendlyPingMesh;
}

UStaticMesh* URadarReturnComponent::GetHostilePingMesh() const
{
	return HostilePingMesh;
}

UStaticMesh* URadarReturnComponent::GetNeutralPingMesh() const
{
	return NeutralPingMesh;
}

UMaterialInterface* URadarReturnComponent::GetFriendlyPingMaterial() const
{
	return FriendlyPingMaterial;
}

UMaterialInterface* URadarReturnComponent::GetHostilePingMaterial() const
{
	return HostilePingMaterial;
}

UMaterialInterface* URadarReturnComponent::GetNeutralPingMaterial() const
{
	return NeutralPingMaterial;
}

bool URadarReturnComponent::IsPrimaryReturn() const
{
	return bIsPrimaryReturn;
}

void URadarReturnComponent::SetIsPrimaryReturn(bool bInIsPrimaryReturn)
{
	bIsPrimaryReturn = bInIsPrimaryReturn;
}

bool URadarReturnComponent::IsActivated() const
{
	return bIsActivated;
}

const TArray<FName>& URadarReturnComponent::GetFactionTags() const
{
	return FactionTags;
}

const TArray<FName>& URadarReturnComponent::GetUnitsTypeTags() const
{
	return UnitsTypeTags;
}

float URadarReturnComponent::GetVisibleRangeMultiplier() const
{
	return VisibleRangeMultiplier;
}

float URadarReturnComponent::GetFriendlyVisibleRangeMultiplier() const
{
	return FriendlyVisibleRangeMultiplier;
}

float URadarReturnComponent::GetHostileVisibleRangeMultiplier() const
{
	return HostileVisibleRangeMultiplier;
}

float URadarReturnComponent::GetNeutralVisibleRangeMultiplier() const
{
	return NeutralVisibleRangeMultiplier;
}

TObjectPtr<USceneComponent> URadarReturnComponent::GetAppliesToComponent() const
{
	return AppliesToComponent;
}

TObjectPtr<USceneComponent> URadarReturnComponent::GetScaleFromComponent() const
{
	return ScaleFromComponent;
}

float URadarReturnComponent::GetFriendlyMeshScale() const 
{
	return FriendlyMeshScale;
}

float URadarReturnComponent::GetHostileMeshScale() const 
{
	return HostileMeshScale;
}

float URadarReturnComponent::GetNeutralMeshScale() const
{
	return NeutralMeshScale;
}

float URadarReturnComponent::GetStandXYScaleMultiplier() const
{
	return StandXYScaleMultiplier;
}

float URadarReturnComponent::GetPingScaleMultiplier() const
{
	return PingScaleMultiplier;
}

float URadarReturnComponent::GetStandZScaleOffset() const
{
	return StandZScaleOffset;
}

bool URadarReturnComponent::ShouldLimitOpacityToRadarDisplay() const
{
	return bShouldLimitOpacityToRadarDisplay;
}

bool URadarReturnComponent::HasReturnStand() const
{
	return bHasReturnStand;
}

bool URadarReturnComponent::ShouldLimitOpacityToRadarDisplayForStand() const
{
	return bShouldLimitOpacityToRadarDisplayForStand;
}

bool URadarReturnComponent::HasReturnPing() const
{
	return bHasReturnPing;
}

bool URadarReturnComponent::ShouldLimitOpacityToRadarDisplayForPing() const
{
	return bShouldLimitOpacityToRadarDisplayForPing;
}

