#include "Character/Laser/AnimNotifyState/AnimNotifyState_LaserBeam.h"
#include "NiagaraComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "NiagaraFunctionLibrary.h"

TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> UAnimNotifyState_LaserBeam::BeamMap1P;
TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> UAnimNotifyState_LaserBeam::BeamMap3P;
TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> UAnimNotifyState_LaserBeam::HitMap1P;
TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> UAnimNotifyState_LaserBeam::HitMap3P;

void UAnimNotifyState_LaserBeam::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef)
{
	if (!MeshComp || !BeamFX || !IsVisibleMesh(MeshComp)) 
        return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner)
        return; 

    Diver = Cast<AUnderwaterCharacter>(Owner);
    if (!Diver) 
        return;

    UWorld* World = MeshComp->GetWorld();
    if (!World || World->IsPreviewWorld()) 
        return;

    FVector CamLoc; 
    FRotator CamRot;
    if (IsFirstPersonMesh(MeshComp))
    {
        // 1P: 카메라 뷰에서 라인트레이스
        AController* Controller = Owner->GetInstigatorController();
        if (!Controller)
            return;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
    }
    else
    {
        // 3P: BaseAimRotation + 메쉬 소켓 위치
        CamRot = Diver->GetBaseAimRotation();
        CamLoc = MeshComp->GetSocketLocation(MuzzleSocket);
    }
    constexpr ECollisionChannel InteractionChannel = ECC_GameTraceChannel4;
    const FVector TraceEnd = CamLoc + CamRot.Vector() * MaxDistance;

    FHitResult Hit;
    World->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, InteractionChannel,
        FCollisionQueryParams(TEXT("LaserTraceCam"), true, Owner));

    const FVector TargetPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
    const FVector MuzzleLoc = MeshComp->GetSocketLocation(MuzzleSocket);
    const FRotator BeamRot = (TargetPoint - MuzzleLoc).Rotation();

    UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, BeamFX, MuzzleLoc, BeamRot,
        FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);

    if (Beam)
    {
        Beam->SetVariableVec3(BeamEndParam, TargetPoint);    // 끝점
        Beam->SetVariableFloat(DurationParam, TotalDuration); // 지속 시간
    }

    UNiagaraComponent* HitEffect = nullptr;
    if (ImpactFX)
    {
        const FRotator ImpactRot = Hit.bBlockingHit ? Hit.ImpactNormal.Rotation() : BeamRot;
        HitEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World, ImpactFX, TargetPoint, ImpactRot);
        HitEffect->SetVariableFloat(DurationParam, TotalDuration);
    }

    if (IsFirstPersonMesh(MeshComp))
    {
        BeamMap1P.Add(MeshComp, Beam);
        HitMap1P.Add(MeshComp, HitEffect);
    }
    else
    {
        BeamMap3P.Add(MeshComp, Beam);
        HitMap3P.Add(MeshComp, HitEffect);
    }
}

void UAnimNotifyState_LaserBeam::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef)
{
    TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>>& BeamMap = IsFirstPersonMesh(MeshComp) ? BeamMap1P : BeamMap3P;
    TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>>& HitMap = IsFirstPersonMesh(MeshComp) ? HitMap1P : HitMap3P;

    if (TWeakObjectPtr<UNiagaraComponent>* BeamPtr = BeamMap.Find(MeshComp))
    {
        if (BeamPtr->IsValid()) BeamPtr->Get()->Deactivate();
        BeamMap.Remove(MeshComp);
    }

    if (TWeakObjectPtr<UNiagaraComponent>* HitPtr = HitMap.Find(MeshComp))
    {
        if (HitPtr->IsValid()) HitPtr->Get()->Deactivate();
        HitMap.Remove(MeshComp);
    }
}

void UAnimNotifyState_LaserBeam::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float FrameDeltaTime, const FAnimNotifyEventReference& EventRef)
{
    if (!IsVisibleMesh(MeshComp)) 
        return;

    TWeakObjectPtr<UNiagaraComponent>* BeamPtr = IsFirstPersonMesh(MeshComp)
        ? BeamMap1P.Find(MeshComp)
        : BeamMap3P.Find(MeshComp);

    if (!BeamPtr || !BeamPtr->IsValid()) return;

    UNiagaraComponent* Beam = BeamPtr->Get();
    UWorld* World = MeshComp->GetWorld();
    AActor* Owner = MeshComp->GetOwner();

    if (!World || !Owner || !Diver) return;

    FVector CamLoc; 
    FRotator CamRot;
    if (IsFirstPersonMesh(MeshComp))
    {
        AController* Controller = Owner->GetInstigatorController();
        if (!Controller)
            return;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
    }
    else
    {
        CamRot = Diver->GetBaseAimRotation();
        CamLoc = MeshComp->GetSocketLocation(MuzzleSocket);
    }

    constexpr ECollisionChannel InteractionChannel = ECC_GameTraceChannel4;
    const FVector TraceEnd = CamLoc + CamRot.Vector() * MaxDistance;
    FHitResult Hit;
    World->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, InteractionChannel,
        FCollisionQueryParams(TEXT("LaserTraceCam"), true, Owner));

    const FVector TargetPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
    const FVector MuzzleLoc = MeshComp->GetSocketLocation(MuzzleSocket);

    Beam->SetVariableVec3(BeamEndParam, TargetPoint);
    Beam->SetWorldLocation(MuzzleLoc);
    Beam->SetWorldRotation((TargetPoint - MuzzleLoc).Rotation());

    TWeakObjectPtr<UNiagaraComponent>* HitPtr = IsFirstPersonMesh(MeshComp) ? HitMap1P.Find(MeshComp) : HitMap3P.Find(MeshComp);
    if (HitPtr && HitPtr->IsValid())
    {
        HitPtr->Get()->SetWorldLocation(TargetPoint);
    }    
}

bool UAnimNotifyState_LaserBeam::IsVisibleMesh(const USkeletalMeshComponent* MeshComp)
{
    AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(MeshComp->GetOwner());
    if (!UnderwaterCharacter)
        return false;

    const bool bLocal = UnderwaterCharacter->IsLocallyControlled();
    return (bLocal && IsFirstPersonMesh(MeshComp)) || (!bLocal && !IsFirstPersonMesh(MeshComp));
}

bool UAnimNotifyState_LaserBeam::IsFirstPersonMesh(const USkeletalMeshComponent* MeshComp)
{
    if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(MeshComp->GetOwner()))
    {
        return MeshComp == UnderwaterCharacter->GetMesh1P();
    }
    return false;
}
