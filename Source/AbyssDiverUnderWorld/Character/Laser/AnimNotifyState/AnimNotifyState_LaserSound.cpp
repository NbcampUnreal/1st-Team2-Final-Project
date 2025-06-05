#include "Character/Laser/AnimNotifyState/AnimNotifyState_LaserSound.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UAnimNotifyState_LaserSound::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef)
{
    if (!MeshComp) return;
    UWorld* World = MeshComp->GetWorld();

#if WITH_EDITOR
    if (!World || World->IsPreviewWorld()) return;
#endif
    AActor* Owner = MeshComp->GetOwner();
    const FVector Loc = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

    if (CachedAudio.IsValid() && CachedAudio->IsPlaying())
    {
        return;
    }
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World, LoopSoundCue, Loc, FRotator::ZeroRotator,
        1.f, 1.f, 0.f, nullptr, nullptr, true);
    if (AudioComp)
    {
        CachedAudio = AudioComp;
    }
}

void UAnimNotifyState_LaserSound::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef)
{
    if (CachedAudio.IsValid())
    {
        CachedAudio->FadeOut(0.15f, 0.f);
    }   
}
