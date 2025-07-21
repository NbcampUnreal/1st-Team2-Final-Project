#include "TutorialTriggerZone.h"
#include "Components/BoxComponent.h"
#include "TutorialManager.h"
#include "Character/UnderwaterCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

ATutorialTriggerZone::ATutorialTriggerZone()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ATutorialTriggerZone::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATutorialTriggerZone::OnOverlapBegin);
        TriggerBox->SetGenerateOverlapEvents(true);
    }

    if (!TutorialManager)
    {
        for (TActorIterator<ATutorialManager> It(GetWorld()); It; ++It)
        {
            TutorialManager = *It;
            break;
        }
    }

    if (!TutorialManager)
    {
        UE_LOG(LogTemp, Error, TEXT("TutorialTriggerZone: TutorialManager not found!"));
    }
}


void ATutorialTriggerZone::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(OtherActor))
    {
        if (!TutorialManager)
        {
            for (TActorIterator<ATutorialManager> It(GetWorld()); It; ++It)
            {
                TutorialManager = *It;
                break;
            }
        }

        if (TutorialManager)
        {
            TutorialManager->PlayCurrentStep(); 
            Destroy(); 
        }
    }
}
