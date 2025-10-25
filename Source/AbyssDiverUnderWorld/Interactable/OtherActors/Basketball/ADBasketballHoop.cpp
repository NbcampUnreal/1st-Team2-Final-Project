#include "Interactable/OtherActors/Basketball/ADBasketballHoop.h"

#include "Interactable/OtherActors/Basketball/ADBasketball.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"

AADBasketballHoop::AADBasketballHoop()
{
 	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	HoopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoopMesh"));
	RootComponent = HoopMesh;
	HoopMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HoopMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	HoopMesh->SetIsReplicated(true);

	GoalZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalZone"));
	GoalZone->SetupAttachment(HoopMesh);
	GoalZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GoalZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GoalZone->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap);
	GoalZone->SetGenerateOverlapEvents(true);

	GoalZone->OnComponentBeginOverlap.AddDynamic(this, &AADBasketballHoop::OnGoalZoneBeginOverlap);
	
	Tags.Add(FName("BasketballHoop"));
}

void AADBasketballHoop::BeginPlay()
{
	Super::BeginPlay();
	
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
	}
}

void AADBasketballHoop::OnGoalZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	LOG(TEXT("[GoalZone Overlap] Authority=%s, Actor=%s, Component=%s, BodyIndex=%d, FromSweep=%d"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		OtherBodyIndex,
		bFromSweep ? 1 : 0);

	if (!HasAuthority())
		return;

	AADBasketball* Basketball = Cast<AADBasketball>(OtherActor);
	if (!Basketball)
		return;

	if (!CanScore())
	{
		LOG(TEXT("Score on cooldown, ignoring"));
		return;
	}

	FVector Velocity = Basketball->GetVelocity();
	if (Velocity.Z > MinimumDownwardVelocity)
	{
		LOG(TEXT("Basketball velocity too low: Z=%.1f (required < %.1f)"),
			Velocity.Z, MinimumDownwardVelocity);
		return;
	}

	LOG(TEXT("GOAL! Basketball scored with velocity: %s"), *Velocity.ToString());

	LastScoreTime = GetWorld()->GetTimeSeconds();
	M_OnScore(Basketball);
}

void AADBasketballHoop::M_OnScore_Implementation(AADBasketball* Basketball)
{
	PlayScoreSound();

	// 효과, 이펙트 등이 추가되면 여기 추가하면 됨
}

void AADBasketballHoop::PlayScoreSound()
{
	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->PlayAt(ESFX::BasketballScore, GetActorLocation());

		LOG(TEXT("Score sound played at %s"),
			*GetActorLocation().ToString());
	}
	else
	{
		LOG(TEXT("SoundSubsystem not available"));
	}
}

bool AADBasketballHoop::CanScore() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastScoreTime) >= ScoreCooldown;
}

USoundSubsystem* AADBasketballHoop::GetSoundSubsystem()
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

