#include "Character/Laser/AnimNotifyState/AnimNotifyState_LaserBeam.h"
#include "NiagaraComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "NiagaraFunctionLibrary.h"

void UAnimNotifyState_LaserBeam::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef)
{
	if (!MeshComp || !BeamFX) return;

    AActor* Owner = MeshComp->GetOwner();
    AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Owner);
    if (!Diver || MeshComp != Diver->GetMesh1P()) return;

    UWorld* World = MeshComp->GetWorld();
    if (!Owner || !World || World->IsPreviewWorld()) return;

    FVector CamLoc; FRotator CamRot;
    AController* Controller = Owner->GetInstigatorController();
    if (!Controller) return;

    Controller->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector TraceEnd = CamLoc + CamRot.Vector() * MaxDistance;

    FHitResult Hit;
    World->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_Visibility,
        FCollisionQueryParams(TEXT("LaserTraceCam"), true, Owner));

    const FVector TargetPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
    const FVector MuzzleLoc = MeshComp->GetSocketLocation(MuzzleSocket);
    const FRotator BeamRot = (TargetPoint - MuzzleLoc).Rotation();

    UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, BeamFX, MuzzleLoc, BeamRot,
        FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);

    if (Beam)
    {
        Beam->SetVariableVec3(BeamEndParam, TargetPoint);    // ³¡Á¡
        Beam->SetVariableFloat(DurationParam, TotalDuration); // Áö¼Ó ½Ã°£
        CachedSpawnedBeam = Beam;
    }

    if (ImpactFX)
    {
        const FRotator ImpactRot = Hit.bBlockingHit ? Hit.ImpactNormal.Rotation() : BeamRot;
        UNiagaraComponent* HitEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World, ImpactFX, TargetPoint, ImpactRot);
        HitEffect->SetVariableFloat(DurationParam, TotalDuration);
        CachedHitEffect = HitEffect;
    }
}

void UAnimNotifyState_LaserBeam::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef)
{
    AActor* Owner = MeshComp->GetOwner();
    AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Owner);
    if (!Diver || MeshComp != Diver->GetMesh1P()) return;

    if (CachedSpawnedBeam.IsValid())
    {
        CachedSpawnedBeam->Deactivate();   // ³²¾Æ ÀÖ´ø ºö ±ò²ûÈ÷ Á¾·á
    }
    if (CachedHitEffect.IsValid())
    {
        CachedHitEffect->Deactivate();
    }
}

void UAnimNotifyState_LaserBeam::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float FrameDeltaTime, const FAnimNotifyEventReference& EventRef)
{
    if (!CachedSpawnedBeam.IsValid()) return;

    UWorld* World = MeshComp->GetWorld();
    AActor* Owner = MeshComp->GetOwner();

    AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Owner);
    if (!Diver || MeshComp != Diver->GetMesh1P()) return;

    AController* Controller = Owner ? Owner->GetInstigatorController() : nullptr;
    if (!World || !Controller) return;

    FVector CamLoc; FRotator CamRot;
    Controller->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector TraceEnd = CamLoc + CamRot.Vector() * MaxDistance;
    FHitResult Hit;
    World->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_Visibility,
        FCollisionQueryParams(TEXT("LaserTraceCam"), true, Owner));

    const FVector NewTargetPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;

    const FVector NewMuzzleLoc = MeshComp->GetSocketLocation(MuzzleSocket);

    CachedSpawnedBeam->SetVariableVec3(BeamEndParam, NewTargetPoint);
    CachedSpawnedBeam->SetWorldRotation((NewTargetPoint - NewMuzzleLoc).Rotation());
    CachedSpawnedBeam->SetWorldLocation(NewMuzzleLoc);
    CachedHitEffect->SetWorldLocation(NewTargetPoint);
}
