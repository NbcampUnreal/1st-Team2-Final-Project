#include "Interactable/OtherActors/Radio/ADRadio.h"
#include "Subsystems/SoundSubsystem.h"
#include "Framework/ADGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AADRadio::AADRadio()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	RadioMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadioMesh"));
	RadioMesh->SetupAttachment(SceneRoot);

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsOn = false;
}
void AADRadio::BeginPlay()
{
	Super::BeginPlay();
	
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}

	if (HasAuthority())
	{
		if (!bIsOn)
		{
			bIsOn = true;
			OnRep_IsOn(); // TurnOn() 호출까지 진행
		}
	}
}

void AADRadio::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		bIsOn = !bIsOn;
		OnRep_IsOn();
	}
}

FString AADRadio::GetInteractionDescription() const
{
	const FText& DisplayText = bIsOn ? DescWhenOn : DescWhenOff;
	return DisplayText.ToString();
}

void AADRadio::OnRep_IsOn()
{
	if (bIsOn)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void AADRadio::TurnOn()
{
	if (!SoundSubsystem.IsValid())
	{
		SoundSubsystem = GetSoundSubsystem();
		if (!SoundSubsystem.IsValid()) { return; }
	}

	if (CurrentBGMId != INDEX_NONE && SoundSubsystem->IsPlaying(CurrentBGMId))
	{
		return;
	}

	CurrentBGMId = SoundSubsystem->PlayAttach(
		RadioBGM,
		RadioMesh,
		RadioVolume,
		bUseFade,
		FadeInTime,
		static_cast<EAudioFaderCurve>(0U)
	);
}

void AADRadio::TurnOff()
{
	if (!SoundSubsystem.IsValid())
	{
		SoundSubsystem = GetSoundSubsystem();
		if (!SoundSubsystem.IsValid()) { return; }
	}

	if (CurrentBGMId != INDEX_NONE)
	{
		SoundSubsystem->StopAudio(
			CurrentBGMId,
			bUseFade,
			FadeOutTime,
			0.0f,
			static_cast<EAudioFaderCurve>(0U)
		);
		CurrentBGMId = INDEX_NONE;
	}
}

void AADRadio::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADRadio, bIsOn);
}

USoundSubsystem* AADRadio::GetSoundSubsystem()
{
	if (SoundSubsystem.IsValid()) { return SoundSubsystem.Get(); }

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem.Get();
	}
	return nullptr;
}

