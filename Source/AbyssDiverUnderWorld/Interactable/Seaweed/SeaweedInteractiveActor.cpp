#include "SeaweedInteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Net/UnrealNetwork.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    SeaweedSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SeaweedSkeletalMesh"));
    SeaweedSkeletalMesh->SetupAttachment(RootComponent);
    SeaweedSkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, -100.f));

    SplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMesh"));
    SplineMesh->SetupAttachment(SceneRoot);
    SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(SceneRoot);
    DetectionSphere->SetSphereRadius(150.f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASeaweedInteractiveActor::BeginPlay()
{
    Super::BeginPlay();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapBegin);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapEnd);

    StartRotation = SeaweedSkeletalMesh->GetRelativeRotation();
    TargetRotation = StartRotation;

    if (SeaweedSkeletalMesh)
    {
        FRotator TargetRot = FRotator(-20.f, 0.f, 0.f); // X축으로 숙이기

        SeaweedSkeletalMesh->SetBoneRotationByName("Bone_004", TargetRot, EBoneSpaces::ComponentSpace);
    }
}

void ASeaweedInteractiveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickRotation(DeltaTime);
    TickSplineBend(DeltaTime);
}

void ASeaweedInteractiveActor::TickRotation(float DeltaTime)
{
    float TargetAlpha = bShouldBend ? 1.f : 0.f;
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, BendSpeed);

    FQuat StartQuat = StartRotation.Quaternion();
    FQuat TargetQuat = TargetRotation.Quaternion();
    FQuat SmoothQuat = FQuat::Slerp(StartQuat, TargetQuat, CurrentAlpha);

    SeaweedMesh->SetRelativeRotation(SmoothQuat);
}

void ASeaweedInteractiveActor::TickSplineBend(float DeltaTime)
{
    float Target = bShouldBend ? 1.f : 0.f;
    LerpAlpha = FMath::FInterpTo(LerpAlpha, Target, DeltaTime, BendSpeed);

    FVector End = FMath::Lerp(
        FVector(0.f, 0.f, 800.f),
        FVector(0.f, BendAmount, 800.f),
        LerpAlpha
    );

    FVector EndTangent = FMath::Lerp(
        FVector(0.f, 0.f, 0.f),
        FVector(0.f, BendAmount * 0.5f, 0.f),
        LerpAlpha
    );

    SplineMesh->SetStartAndEnd(FVector(0.f, 0.f, 0.f), FVector::ZeroVector, End, EndTangent);
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
            FVector PlayerToSeaweed = GetActorLocation() - OtherActor->GetActorLocation();
            PlayerToSeaweed.Z = 0.f;

            if (!PlayerToSeaweed.IsNearlyZero())
            {
                PlayerToSeaweed.Normalize();

                FVector PitchAxis = FVector::CrossProduct(FVector::UpVector, PlayerToSeaweed).GetSafeNormal();
                if (PitchAxis.IsNearlyZero())
                {
                    PitchAxis = FVector::RightVector;
                }

                float Radians = FMath::DegreesToRadians(BendPitch);
                FQuat BendQuat = FQuat(PitchAxis, -Radians);

                TargetRotation = (BendQuat * StartRotation.Quaternion()).Rotator();
            }

            bShouldBend = true;
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
            ForceNetUpdate();
        }
    }
}

void ASeaweedInteractiveActor::OnRep_BendState()
{
    // (클라이언트용) 회전 방향 적용은 Tick에서 처리
}

void ASeaweedInteractiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASeaweedInteractiveActor, bShouldBend);
    DOREPLIFETIME(ASeaweedInteractiveActor, TargetRotation);
}
