#include "Interface/IADInteractable.h"

void IIADInteractable::Interact_Implementation(AActor* InstigatorActor)
{
}

void IIADInteractable::InteractHold_Implementation(AActor* InstigatorActor)
{

}

void IIADInteractable::OnHoldStart_Implementation(APawn* InstigatorPawn)
{

}

void IIADInteractable::OnHoldStop_Implementation(APawn* InstigatorPawn)
{
}

float IIADInteractable::GetHoldDuration_Implementation(AActor* InstigatorActor) const
{
    return 0.0f;
}

FString IIADInteractable::GetInteractionDescription() const
{
    return TEXT("None");
}
