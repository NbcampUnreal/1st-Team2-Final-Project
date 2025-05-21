#include "Interactable/OtherActors/Tablet/ADTablet.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "UI/TabletBaseWidget.h"

// Sets default values
AADTablet::AADTablet()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	TabletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TabletMesh"));
	TabletMesh->SetupAttachment(SceneRoot);
	TabletMesh->SetSimulatePhysics(true);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TabletMesh->SetMobility(EComponentMobility::Movable);
	TabletMesh->SetIsReplicated(true);

	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(TabletMesh);
	ScreenWidget->SetVisibility(false);
	ScreenWidget->SetIsReplicated(true);

	ScreenWidget->SetDrawAtDesiredSize(false);
	ScreenWidget->SetRelativeScale3D({ 0.1f,0.1f, 0.1f });
	ScreenWidget->SetPivot({ 0.5f, 0.5f });

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsHeld = false;

}

void AADTablet::Interact_Implementation(AActor* InstigatorActor)
{
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorActor);
	
	if (!UnderwaterCharacter) return;
	LOG(TEXT("Is Character"))

	if (!bIsHeld)
	{
		Pickup(UnderwaterCharacter);
	}
	else
	{
		PutDown();
	}    
}

void AADTablet::Pickup(AUnderwaterCharacter* UnderwaterCharacter)
{
	if (!UnderwaterCharacter) return;


	TabletMesh->SetSimulatePhysics(false);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TabletMesh->SetVisibility(false, true);

	USkeletalMeshComponent* Mesh1P = UnderwaterCharacter->GetMesh1P();
	if (!Mesh1P) return;

	TabletMesh->AttachToComponent(
		Mesh1P,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TEXT("TabletSocket")
	);
	bIsHeld = true;

	TabletMesh->SetRelativeLocation(FVector::ZeroVector);
	TabletMesh->SetRelativeRotation(FRotator::ZeroRotator);

	ScreenWidget->SetVisibility(true);

	if (UUserWidget* UserWidget = ScreenWidget->GetUserWidgetObject())
	{
		if (UTabletBaseWidget* TabletWidget = Cast<UTabletBaseWidget>(UserWidget))
		{
			if (TabletWidget->Start)
			{
				// PlayAnimation(애니메이션, 시작 시간, LoopCount, 플레이 모드)
				TabletWidget->PlayAnimation(TabletWidget->Start, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
				LOG(TEXT("TabletUI Animation Start"))
			}
		}
	}

	LOG(TEXT("Tablet attached to Mesh1P at TabletSocket"));
	
	HeldBy = UnderwaterCharacter;
}

void AADTablet::PutDown()
{

	TabletMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	
	TabletMesh->SetVisibility(true, true);

	TabletMesh->SetSimulatePhysics(true);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	bIsHeld = false;

	ScreenWidget->SetVisibility(false);

	LOG(TEXT("Tablet detached and placed back"));

	HeldBy = nullptr;
}

void AADTablet::OnRep_Held()
{
	if (bIsHeld)
	{
		if (HeldBy)
		{
			Pickup(HeldBy);
			LOG(TEXT("Pickup!!"));
			ScreenWidget->InitWidget();
			ScreenWidget->MarkRenderStateDirty();
		}
	}
	else
	{
		PutDown();
	}
}

void AADTablet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADTablet, bIsHeld);
	DOREPLIFETIME(AADTablet, HeldBy);
}



