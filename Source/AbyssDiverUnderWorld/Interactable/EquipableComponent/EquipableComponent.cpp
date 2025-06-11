#include "Interactable/EquipableComponent/EquipableComponent.h"
#include "EquipableComponent.h"


UEquipableComponent::UEquipableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UEquipableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	USkeletalMeshComponent* SkelComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (SkelComp)
	{
		MeshComponent = SkelComp;
		SkeletalMeshAsset = SkelComp->SkeletalMesh;
		return;
	}
	UStaticMeshComponent* StatComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	if (StatComp)
	{
		MeshComponent = StatComp;
		StaticMeshAsset = StatComp->GetStaticMesh();
		return;
	}
	
}

bool UEquipableComponent::IsSkeletal() const
{
	return MeshComponent.IsValid() && Cast<USkeletalMeshComponent>(MeshComponent.Get()) != nullptr;
}

bool UEquipableComponent::IsStatic() const
{
	return MeshComponent.IsValid() && Cast<UStaticMeshComponent>(MeshComponent.Get()) != nullptr;
}


