#include "TutorialTriggerZone.h"
#include "Components/BoxComponent.h"
#include "TutorialManager.h"
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
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        if (TutorialManager)
        {
            TutorialManager->PlayCurrentStep();
            Destroy(); // 한 번만 실행되도록 제거
        }
    }
}
