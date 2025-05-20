#include "Interactable/OtherActors/Tablet/ADTablet.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "AbyssDiverUnderWorld.h"

// Sets default values
AADTablet::AADTablet()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	TabletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TabletMesh"));
	TabletMesh->SetupAttachment(SceneRoot);
	TabletMesh->SetSimulatePhysics(true);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(TabletMesh);
	ScreenWidget->SetVisibility(false);

	ScreenWidget->SetDrawAtDesiredSize(false);
	ScreenWidget->SetRelativeScale3D({ 0.1f,0.1f, 0.1f });
	ScreenWidget->SetPivot({ 0.5f, 0.5f });

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsHeld = false;
}

void AADTablet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bIsHeld || !OwnerCamera) return;

	// ī�޶� ��ġ + ���溤�� * �Ÿ� + ������
	FVector CamLoc = OwnerCamera->GetComponentLocation();
	FVector CamFwd = OwnerCamera->GetForwardVector();
	FRotator CamRot = OwnerCamera->GetComponentRotation();

	FVector TargetLoc = CamLoc + CamFwd * HoldOffsetLocation.X
		+ CamRot.RotateVector(FVector(0, HoldOffsetLocation.Y, HoldOffsetLocation.Z));
	FRotator TargetRot = CamRot + HoldOffsetRotation;

	SetActorLocationAndRotation(TargetLoc, TargetRot);
}

void AADTablet::Interact_Implementation(AActor* InstigatorActor)
{
	if (bIsHeld) { PutDown(); }
	else { Pickup(InstigatorActor); }
}

void AADTablet::Pickup(AActor* InstigatorActor)
{
	if (!InstigatorActor) return;
	if (bIsHeld) return;

	// ����ġ ����
	CachedWorldTransform = GetActorTransform();

	// ī�޶� ������Ʈ ��������
	if (APawn* Pawn = Cast<APawn>(InstigatorActor))
	{
		if (UCameraComponent* Cam = Pawn->FindComponentByClass<UCameraComponent>())
		{
			OwnerCamera = Cam;
			LOG(TEXT("%s"), *Cam->GetName());
			bIsHeld = true;

			// Physics, Collision ����(����)
			TabletMesh->SetSimulatePhysics(false);
			TabletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Tick Ȱ��ȭ
			SetActorTickEnabled(true);

			LOG(TEXT("Hold!!"));
		}
	}
}

void AADTablet::PutDown()
{
	if (!bIsHeld) return;

	bIsHeld = false;
	SetActorTickEnabled(false);

	// ���� ��ġ ����
	SetActorTransform(CachedWorldTransform);

	// Physics, Collision ����(����)
	TabletMesh->SetSimulatePhysics(true);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	OwnerCamera = nullptr;
}



