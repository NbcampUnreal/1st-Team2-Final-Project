// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/OtherActors/Level/InteractableButton.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"

AInteractableButton::AInteractableButton()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultComp = CreateDefaultSubobject<USceneComponent>(TEXT("Default Comp"));
	SetRootComponent(DefaultComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	MeshComp->SetupAttachment(RootComponent);

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("Interactable Comp"));

	bReplicates = true;
	SetReplicateMovement(true);
}


void AInteractableButton::Interact_Implementation(AActor* InstigatorActor)
{
	if (APlayerController* PC = Cast<APlayerController>(InstigatorActor->GetInstigatorController()))
	{
		if (!PC->IsLocalController()) return;

		OnButtonPressed.ExecuteIfBound(InstigatorActor);
		LOGV(Error, TEXT("ButtonInteract"));
	}
}

void AInteractableButton::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInteractableButton, ButtonAction);
	DOREPLIFETIME(AInteractableButton, ReplicatedMesh);
}

void AInteractableButton::OnRep_SetMesh()
{
	MeshComp->SetStaticMesh(ReplicatedMesh);
}

void AInteractableButton::SetStaticMesh(UStaticMesh* Mesh)
{
	ReplicatedMesh = Mesh;
	MeshComp->SetStaticMesh(Mesh);
}

UADInteractableComponent* AInteractableButton::GetInteractableComponent() const
{
	return InteractableComp;
}

FString AInteractableButton::GetInteractionDescription() const
{
	return ButtonDescription;
}

bool AInteractableButton::IsHoldMode() const
{
	return false;
}

void AInteractableButton::SetButtonDescription(const FString& Description)
{
	if (HasAuthority())
	{
		ButtonDescription = Description;
	}
	else
	{
		ButtonDescription = TEXT("호스트만 조작 가능합니다.");
	}
}

