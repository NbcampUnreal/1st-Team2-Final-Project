#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Interface/IADInteractable.h"

DEFINE_LOG_CATEGORY(ItemComponentLog);

UADInteractableComponent::UADInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UADInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		TArray<UMeshComponent*> Found;
		Owner->GetComponents<UMeshComponent>(Found);

		for (UMeshComponent* Mesh : Found)
		{
			Mesh->SetRenderCustomDepth(false);
			Mesh->SetCustomDepthStencilValue(CustomStencilValue);
			MeshComponents.Add(Mesh);
		}
	}
}

void UADInteractableComponent::SetHighLight(bool bEnable)
{
	for (UMeshComponent* Mesh : MeshComponents)
	{
		if (Mesh)
		{
			Mesh->SetRenderCustomDepth(bEnable);
		}
	}
	bHighlighted = bEnable;
}

void UADInteractableComponent::Interact(AActor* InstigatorActor)
{
	if (!InstigatorActor || !GetOwner()->HasAuthority())
		return;

	if (IIADInteractable* Inter = Cast<IIADInteractable>(GetOwner()))
	{
		IIADInteractable::Execute_Interact(GetOwner(), InstigatorActor);
	}		
}



