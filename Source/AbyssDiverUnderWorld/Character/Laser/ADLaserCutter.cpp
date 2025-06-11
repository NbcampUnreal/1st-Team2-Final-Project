#include "Character/Laser/ADLaserCutter.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/EquipableComponent/EquipableComponent.h"

// Sets default values
AADLaserCutter::AADLaserCutter()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = SceneComp;

    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StaticMeshComp"));
    SkeletalMeshComp->SetupAttachment(SceneComp);
    EquipableComp = CreateDefaultSubobject<UEquipableComponent>(TEXT("EquipableComponent"));
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
            SkeletalMeshComp->SetVisibility(bLocallyControlled, true);
        }
        else
        {
            // 3P 메쉬: 내 클라이언트에서는 숨기고, 다른 클라이언트에서는 보이게
            SkeletalMeshComp->SetVisibility(!bLocallyControlled, true);
        }
    }
}
