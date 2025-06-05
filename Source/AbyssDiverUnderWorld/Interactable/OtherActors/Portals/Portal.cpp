#include "Interactable/OtherActors/Portals/Portal.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Framework/ADInGameMode.h"
#include "Framework/ADGameInstance.h"
#include "AbyssDiverUnderWorld.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "Subsystems/SoundSubsystem.h"


APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultComp = CreateDefaultSubobject<USceneComponent>(TEXT("Default"));
	SetRootComponent(DefaultComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsHoldMode = false;
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	if (TaregetDestination == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("포탈에 목표지점을 등록해주세요 레벨상에서"));
		LOGVN(Error, TEXT("TaregetDestination == nullptr"));
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}
	GetSoundSubsystem()->PlayAt(ESFX::Submarine, GetActorLocation(), 2.0f);
}

void APortal::Interact_Implementation(AActor* InstigatorActor)
{
	LOGVN(Warning, TEXT("Portal Interacting.."));
	if (IsConditionMet() == false)
	{
		LOGVN(Warning, TEXT("Condition is not met"));
		return;
	}

	InstigatorActor->SetActorLocation(TaregetDestination->GetActorLocation());
}

bool APortal::IsConditionMet()
{
	if (HasAuthority() == false)
	{
		return false;
	}

	if (TaregetDestination == nullptr)
	{
		LOGVN(Error, TEXT("TaregetDestination == nullptr"));
		return false;
	}

	AADInGameMode* GM = Cast<AADInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM == nullptr)
	{
		LOGVN(Warning, TEXT("GM == nullptr"));
		return false;
	}
	
	return GM->IsAllPhaseCleared();
}

UADInteractableComponent* APortal::GetInteractableComponent() const
{
	return InteractableComp;
}

bool APortal::IsHoldMode() const
{
	return false;
}

FString APortal::GetInteractionDescription() const
{
	return TEXT("Go to Submarine!");
}

USoundSubsystem* APortal::GetSoundSubsystem()
{
	if (IsValid(SoundSubsystem))
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	check(SoundSubsystem);
	return SoundSubsystem;
}


