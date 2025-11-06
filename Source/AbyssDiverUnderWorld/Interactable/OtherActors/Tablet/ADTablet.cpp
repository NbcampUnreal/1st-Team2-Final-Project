#include "Interactable/OtherActors/Tablet/ADTablet.h"

#include "Character/UnderwaterCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "UI/TabletBaseWidget.h"
#include "Framework/ADGameInstance.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/Localizations/LocalizationSubsystem.h"

#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"  

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
}

void AADTablet::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
	}
}

void AADTablet::Interact_Implementation(AActor* InstigatorActor)
{
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorActor);

	if (!UnderwaterCharacter) return;

	if (!HeldBy)
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
	if (!UnderwaterCharacter || !UnderwaterCharacter->GetMesh1P()) return;
	USkeletalMeshComponent* Mesh1P = UnderwaterCharacter->GetMesh1P();

	HeldBy = UnderwaterCharacter;
	HeldByWeakPtr = UnderwaterCharacter;
	
	TabletMesh->SetSimulatePhysics(false);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TabletMesh->SetVisibility(false, true);
	
	UCameraComponent* Camera1P = UnderwaterCharacter->GetFirstPersonCameraComponent();
	if (Camera1P)
	{
		TabletMesh->AttachToComponent(
			Camera1P,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			NAME_None
		);

		TabletMesh->SetRelativeLocation(HoldOffsetLocation);
		TabletMesh->SetRelativeRotation(HoldOffsetRotation);
		LOG(TEXT("Camera1P"));
	}
	else
	{
		if (Mesh1P)
		{
			TabletMesh->AttachToComponent(
				Mesh1P,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("TabletSocket")
			);
			TabletMesh->SetRelativeLocation(HoldOffsetLocation);
			TabletMesh->SetRelativeRotation(HoldOffsetRotation);
		}
		LOG(TEXT("No Camera1P"));
	}

	//TabletMesh->AttachToComponent(
	//	Mesh1P,
	//	FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	//	TEXT("TabletSocket")
	//);
	//TabletMesh->SetRelativeLocation(HoldOffsetLocation);
	//TabletMesh->SetRelativeRotation(HoldOffsetRotation);

	ScreenWidget->SetVisibility(true);
	if (UUserWidget* UserWidget = ScreenWidget->GetUserWidgetObject())
	{
		if (UTabletBaseWidget* TabletWidget = Cast<UTabletBaseWidget>(UserWidget))
		{
			if (TabletWidget->Start)
			{
				if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
				{
					SoundSubsystem->PlayAt(ESFX::OpenTablet, GetActorLocation());
				}
				
				// PlayAnimation(애니메이션, 시작 시간, LoopCount, 플레이 모드)
				TabletWidget->PlayAnimation(TabletWidget->Start, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
			}
		}
	}

	HeldBy->OnEmoteStartDelegate.AddDynamic(this, &AADTablet::OnEmoteStart);
	HeldBy->OnEmoteEndDelegate.AddDynamic(this, &AADTablet::OnEmoteEnd);
	
	LOG(TEXT("Tablet attached to Mesh at TabletSocket"));
}

void AADTablet::PutDown()
{
	if (HeldByWeakPtr.IsValid())
	{
		HeldByWeakPtr.Get()->OnEmoteStartDelegate.RemoveDynamic(this, &AADTablet::OnEmoteStart);
		HeldByWeakPtr.Get()->OnEmoteEndDelegate.RemoveDynamic(this, &AADTablet::OnEmoteEnd);
	}
	
	HeldBy = nullptr;
	HeldByWeakPtr = nullptr;
	
	TabletMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	TabletMesh->SetVisibility(true, true);
	TabletMesh->SetSimulatePhysics(true);
	TabletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	ScreenWidget->SetVisibility(false);
	
	LOG(TEXT("Tablet detached and placed back"));
}

void AADTablet::OnRep_HeldBy()
{
	if (HeldBy)
	{
		LOG(TEXT("OnRep_HeldBy - Picking up tablet"));
		Pickup(HeldBy);
		// 추후 문제가 생기지 않는다면 제거
		// ScreenWidget->InitWidget();
		// ScreenWidget->MarkRenderStateDirty();
	}
	else
	{
		PutDown();
	}
}

void AADTablet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADTablet, HeldBy);
}

void AADTablet::OnEmoteStart()
{
	if (IsValid(HeldBy))
	{
		ScreenWidget->SetVisibility(false);
	}
}

void AADTablet::OnEmoteEnd()
{
	if (IsValid(HeldBy))
	{
		ScreenWidget->SetVisibility(true);
	}
}

FString AADTablet::GetInteractionDescription() const
{
	ULocalizationSubsystem* LocalizationSubsystem = GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
	if (IsValid(LocalizationSubsystem) == false)
	{
		LOGV(Error, TEXT("Cant Get LocalizationSubsystem"));
		return "";
	}

	return LocalizationSubsystem->GetLocalizedText(ST_InteractionDescription::TableKey, ST_InteractionDescription::Tablet_Pickup).ToString();
}

USoundSubsystem* AADTablet::GetSoundSubsystem()
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UADGameInstance* GameInstance = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
		{
			SoundSubsystemWeakPtr = GameInstance->GetSubsystem<USoundSubsystem>();
		}
	}

	return SoundSubsystemWeakPtr.Get();
}



