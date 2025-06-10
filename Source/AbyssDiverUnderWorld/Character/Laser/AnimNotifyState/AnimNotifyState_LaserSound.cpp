#include "Character/Laser/AnimNotifyState/AnimNotifyState_LaserSound.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

TMap<USkeletalMeshComponent*, TWeakObjectPtr<UAudioComponent>> UAnimNotifyState_LaserSound::SoundMap;

void UAnimNotifyState_LaserSound::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef)
{
    if (!MeshComp) return;
    UWorld* World = MeshComp->GetWorld();

#if WITH_EDITOR
    if (!World || World->IsPreviewWorld()) return;
#endif
    if (TWeakObjectPtr<UAudioComponent>* ExistingSound = SoundMap.Find(MeshComp))
    {
        if (ExistingSound->IsValid() && ExistingSound->Get()->IsPlaying())
            return;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
        LoopSoundCue,
        MeshComp,                    
        NAME_None,                  // 소켓이 없으면 NAME_None
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        true                         // AutoDestroy
    );
    if (AudioComp)
    {
        SoundMap.Add(MeshComp, AudioComp);
    }
}

void UAnimNotifyState_LaserSound::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef)
{
    if (TWeakObjectPtr<UAudioComponent>* AudioPtr = SoundMap.Find(MeshComp))
    {
        if (AudioPtr->IsValid())
            AudioPtr->Get()->FadeOut(0.15f, 0.f);
        SoundMap.Remove(MeshComp);
    }
}