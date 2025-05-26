#include "Character/Laser/ADLaserCutter.h"
#include "Character/UnderwaterCharacter.h"

// Sets default values
AADLaserCutter::AADLaserCutter()
{
	PrimaryActorTick.bCanEverTick = false;

    SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = SceneComp;

    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
    StaticMeshComp->SetupAttachment(SceneComp);
}

void AADLaserCutter::M_SetupVisibility_Implementation(bool bIs1P)
{
    SetupVisibility(bIs1P);
}

void AADLaserCutter::SetupVisibility(bool bIs1P)
{
    if (AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(GetOwner()))
    {
        bool bLocallyControlled = Diver->IsLocallyControlled();

        if (bIs1P)
        {
            // 1P 메쉬: 내 클라이언트만 보이게
            StaticMeshComp->SetVisibility(bLocallyControlled, true);
        }
        else
        {
            // 3P 메쉬: 내 클라이언트에서는 숨기고, 다른 클라이언트에서는 보이게
            StaticMeshComp->SetVisibility(!bLocallyControlled, true);
        }
    }
}
