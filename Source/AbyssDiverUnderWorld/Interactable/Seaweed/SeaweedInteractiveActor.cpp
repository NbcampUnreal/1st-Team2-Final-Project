#include "SeaweedInteractiveActor.h"
#include "Components/SphereComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Net/UnrealNetwork.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    SeaweedMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("SeaweedMesh"));
    SeaweedMesh->SetupAttachment(SceneRoot);
    SeaweedMesh->SetRelativeLocation(FVector(0.f, 0.f, -100.f));

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(SceneRoot);
    DetectionSphere->SetSphereRadius(100.f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASeaweedInteractiveActor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false); // Tick 비활성화 시작

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapBegin);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapEnd);

    for (const FName& BoneName : BendingBoneNames)
    {
        if (!SeaweedMesh->DoesSocketExist(BoneName)) continue;
        FTransform BoneTransform = SeaweedMesh->GetBoneTransformByName(BoneName, EBoneSpaces::ComponentSpace);
        OriginalBoneRotations.Add(BoneName, BoneTransform.GetRotation());
    }
}

void ASeaweedInteractiveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const float TargetAlpha = bShouldBend ? 1.f : 0.f;
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, BendSpeed);

    const int32 NumBones = BendingBoneNames.Num();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const FName& BoneName = BendingBoneNames[i];
        if (!OriginalBoneRotations.Contains(BoneName)) continue;

        float Ratio = static_cast<float>(i + 1) / NumBones;
        float CurveAlpha = FMath::SmoothStep(0.f, 1.f, Ratio);
        CurveAlpha = FMath::Pow(CurveAlpha, 1.5f);

        float Angle = CurveAlpha * CurrentAlpha * BendAmount;

        FQuat AddRot = FQuat(BendAxis, FMath::DegreesToRadians(Angle));
        FQuat BaseRot = OriginalBoneRotations[BoneName];
        FQuat FinalRot = AddRot * BaseRot;

        FTransform BoneTransform = SeaweedMesh->GetBoneTransformByName(BoneName, EBoneSpaces::ComponentSpace);
        BoneTransform.SetRotation(FinalRot);
        SeaweedMesh->SetBoneTransformByName(BoneName, BoneTransform, EBoneSpaces::ComponentSpace);
    }

    // ✅ 목표에 도달하면 Tick 비활성화
    if (FMath::IsNearlyEqual(CurrentAlpha, TargetAlpha, 0.01f))
    {
        SetActorTickEnabled(false);
    }
}

void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    if (Cast<AUnderwaterCharacter>(OtherActor))
    {
        OverlappingCharacterCount++;

        if (OverlappingCharacterCount == 1)
        {
            FVector Dir = (OtherActor->GetActorLocation() - GetActorLocation());
            Dir.Z = 0.f;
            Dir.Normalize();

            FVector PitchAxis = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();
            if (PitchAxis.IsNearlyZero()) PitchAxis = FVector::RightVector;

            BendAxis = PitchAxis;
            bShouldBend = true;

            SetActorTickEnabled(true); // ✅ 휘어짐 시작 → Tick 활성화
            ForceNetUpdate();
        }
    }
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority()) return;

    if (Cast<AUnderwaterCharacter>(OtherActor))
    {
        OverlappingCharacterCount = FMath::Max(0, OverlappingCharacterCount - 1);
        if (OverlappingCharacterCount == 0)
        {
            bShouldBend = false;
            SetActorTickEnabled(true); // ✅ 되돌아가기 시작 → Tick 유지
            ForceNetUpdate();
        }
    }
}

void ASeaweedInteractiveActor::OnRep_BendState() {
    SetActorTickEnabled(true);
}

void ASeaweedInteractiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASeaweedInteractiveActor, bShouldBend);
    DOREPLIFETIME(ASeaweedInteractiveActor, BendAxis);
}
