#include "Interactable/Item/Component/AnimNotify/AnimNotify_SwingHammerHit.h"
#include "Kismet/GameplayStatics.h"

UAnimNotify_SwingHammerHit::UAnimNotify_SwingHammerHit()
{
#if WITH_EDITOR       // 에디터 빌드에서만
    bDebugDraw = true;
#else                
    bDebugDraw = false;
#endif
}

void UAnimNotify_SwingHammerHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    // 데미지는 서버에서만
    if (!Owner->HasAuthority()) return;

    // 짧은 사거리
    FVector EyeLoc; FRotator EyeRot;
    Owner->GetActorEyesViewPoint(EyeLoc, EyeRot);
    const FVector Fwd = EyeRot.Vector();
    const FVector Start = EyeLoc + Fwd * 40.f;
    const FVector End = Start + Fwd * Range;

    // 스윕: Pawn + Monster Channel
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SwingHammer_Notify), false, Owner);
    Params.AddIgnoredActor(Owner);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(MonsterChannel);

    const bool bHit = Owner->GetWorld()->SweepMultiByObjectType(
        Hits, Start, End, FQuat::Identity, ObjParams,
        FCollisionShape::MakeSphere(Radius), Params
    );

    // 디버그
    if (bDebugDraw)
    {
        const FVector Dir = (End - Start);
        const float   Dist = Dir.Size();
        const FVector Mid = (Start + End) * 0.5f;
        const FQuat   Rot = FRotationMatrix::MakeFromZ(Dir.GetSafeNormal()).ToQuat();

        DrawDebugCapsule(Owner->GetWorld(), Mid, Dist * 0.5f, Radius, Rot,
            bHit ? FColor::Green : FColor::Red, false, 1.5f, 0, 2.f);
        DrawDebugSphere(Owner->GetWorld(), Start, Radius, 12, FColor::Blue, false, 1.5f, 0, 1.5f);
        DrawDebugSphere(Owner->GetWorld(), End, Radius, 12, FColor::Blue, false, 1.5f, 0, 1.5f);

        for (const FHitResult& H : Hits)
        {
            DrawDebugPoint(Owner->GetWorld(),
                H.ImpactPoint.IsNearlyZero() ? H.Location : H.ImpactPoint,
                14.f, FColor::Yellow, false, 1.5f, 0);
        }
    }

    if (!bHit) return;

    TSet<AActor*> UniqueActors; UniqueActors.Reserve(Hits.Num());
    for (const FHitResult& H : Hits)
    {
        AActor* Target = H.GetActor();
        if (!Target || Target == Owner) continue;
        if (bIgnoreBoss && Target->ActorHasTag(BossTag)) continue;
        if (UniqueActors.Contains(Target)) continue;
        UniqueActors.Add(Target);

        UGameplayStatics::ApplyDamage(
            Target, Damage,
            Owner->GetInstigatorController(), Owner,
            UDamageType::StaticClass()
        );
    }
}
