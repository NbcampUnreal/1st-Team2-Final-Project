#include "Interactable/Item/Component/AnimNotify/AnimNotify_PunchHit.h"
#include "Kismet/GameplayStatics.h"

UAnimNotify_PunchHit::UAnimNotify_PunchHit()
{
#if WITH_EDITOR
    bDebugDraw = true;
#else
    bDebugDraw = false;
#endif
}

void UAnimNotify_PunchHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
        return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner || !Owner->HasAuthority())
        return;

    FVector EyeLoc;
    FRotator EyeRot;
    Owner->GetActorEyesViewPoint(EyeLoc, EyeRot);
    const FVector ForwardVec = EyeRot.Vector();
    const FVector Start = EyeLoc + ForwardVec * 30.f;
    const FVector End = Start + ForwardVec * Range;

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(Punch_Notify), false, Owner);
    Params.AddIgnoredActor(Owner);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(MonsterChannel);

    const bool bHit = Owner->GetWorld()->SweepMultiByObjectType(
        Hits, Start, End, FQuat::Identity, ObjParams,
        FCollisionShape::MakeSphere(Radius), Params
    );

    // 디버그 라인(에디터 에서만 출력)
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
                12.f, FColor::Yellow, false, 1.5f, 0);
        }
    }

    if (!bHit)
        return;

    // 액터 단위 중복 방지
    TSet<AActor*> UniqueActors;
    UniqueActors.Reserve(Hits.Num());

    for (const FHitResult& Hit : Hits)
    {
        AActor* Target = Hit.GetActor();
        if (!Target || Target == Owner)
            continue;
        if (UniqueActors.Contains(Target))
            continue;

        UniqueActors.Add(Target);
        UGameplayStatics::ApplyDamage(
            Target,
            Damage,
            Owner->GetInstigatorController(),
            Owner,
            UDamageType::StaticClass()
        );
    }
}
