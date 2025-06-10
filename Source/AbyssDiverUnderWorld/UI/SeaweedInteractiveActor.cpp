#include "SeaweedInteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UnderwaterCharacter.h"
#include "Net/UnrealNetwork.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

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

    BottomSplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("BottomSplineMesh"));
    BottomSplineMesh->SetupAttachment(SceneRoot);
    BottomSplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TopSplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("TopSplineMesh"));
    TopSplineMesh->SetupAttachment(SceneRoot);
    TopSplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
        // 하단: 0 ~ 400
        BottomSplineMesh->SetStaticMesh(SeaweedMeshAsset);
        BottomSplineMesh->SetStartAndEnd(
            FVector(0.f, 0.f, 0.f),
            FVector(0.f, 0.f, 0.f),
            FVector(0.f, 0.f, 400.f),
            FVector(0.f, 0.f, 0.f)
        );

        // 상단: 400 ~ 800 (변형 대상)
        TopSplineMesh->SetStaticMesh(SeaweedMeshAsset);
        TopSplineMesh->SetStartAndEnd(
            FVector(0.f, 0.f, 400.f),
            FVector(0.f, 0.f, 0.f),
            FVector(0.f, 0.f, 800.f),
            FVector(0.f, 0.f, 0.f)
        );
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

    // Start: 고정점 (하단 끝 위치)
    FVector Start = FVector(0.f, 0.f, 400.f);

    // End: 상단 끝 위치 + 휘는 방향
    FVector End = FMath::Lerp(
        FVector(0.f, 0.f, 800.f),
        FVector(0.f, BendAmount, 800.f),
        LerpAlpha
    );

    // Tangents
    FVector StartBendTangent = FVector(0.f, 0.f, 0.f);
    FVector EndBendTangent = FMath::Lerp(
        FVector(0.f, 0.f, 0.f),
        FVector(0.f, BendAmount * 0.5f, 0.f),
        LerpAlpha
    );

    TopSplineMesh->SetStartAndEnd(Start, StartBendTangent, End, EndBendTangent);
}


void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    if (Cast<AUnderwaterCharacter>(OtherActor))
    {
        OverlappingCharacterCount++;

        // ✅ 처음 들어온 캐릭터일 때만 방향 계산
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
    //if (!bShouldBend)
   // {
   //     TargetRotation = StartRotation;
    //}
}

void ASeaweedInteractiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASeaweedInteractiveActor, bShouldBend);
    DOREPLIFETIME(ASeaweedInteractiveActor, TargetRotation);
}
