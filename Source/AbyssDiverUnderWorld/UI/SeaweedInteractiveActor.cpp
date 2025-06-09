#include "SeaweedInteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UnderwaterCharacter.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    SeaweedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeaweedMesh"));
    SeaweedMesh->SetupAttachment(SceneRoot);
    SeaweedMesh->SetRelativeLocation(FVector(0.f, 0.f, -100.f));

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

    StartRotation = SeaweedMesh->GetRelativeRotation();
    TargetRotation = StartRotation;

    if (SeaweedMeshAsset)
    {
        SplineMesh->SetStaticMesh(SeaweedMeshAsset);
        SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, StartTangent);
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

    // 더 부드럽게 보간
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, BendSpeed);

    // 부드러운 회전 보간 (Quaternion)
    FQuat StartQuat = StartRotation.Quaternion();
    FQuat TargetQuat = TargetRotation.Quaternion();
    FQuat SmoothQuat = FQuat::Slerp(StartQuat, TargetQuat, CurrentAlpha);

    SeaweedMesh->SetRelativeRotation(SmoothQuat);
}


void ASeaweedInteractiveActor::TickSplineBend(float DeltaTime)
{
    float Target = bShouldBend ? 1.f : 0.f;
    LerpAlpha = FMath::FInterpTo(LerpAlpha, Target, DeltaTime, BendSpeed * 0.5f);

    FVector CurEnd = FMath::Lerp(EndPos, EndPos + FVector(BendAmount * 0.3f, BendAmount * 0.3f, 0.f), LerpAlpha);
    FVector CurTangent = FMath::Lerp(StartTangent, FVector(BendAmount * 0.5f, BendAmount * 0.5f, 100.f), LerpAlpha);

    SplineMesh->SetStartAndEnd(StartPos, StartTangent, CurEnd, CurTangent);
}

void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<AUnderwaterCharacter>(OtherActor))
    {
        OverlappingCharacterCount++;

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
            bShouldBend = true;
        }
    }
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<AUnderwaterCharacter>(OtherActor))
    {
        OverlappingCharacterCount = FMath::Max(0, OverlappingCharacterCount - 1);
        if (OverlappingCharacterCount == 0)
        {
            bShouldBend = false;
        }
    }
}
