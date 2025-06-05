#include "SeaweedInteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // Static Mesh (시각적 회전용)
    SeaweedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeaweedMesh"));
    SeaweedMesh->SetupAttachment(SceneRoot);
    SeaweedMesh->SetRelativeLocation(FVector(0.f, 0.f, -100.f));

    // Spline Mesh (휘는 효과용)
    SplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMesh"));
    SplineMesh->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/OceanFloor/Meshes/SM_SeaweedE.SM_SeaweedE"));
    if (MeshAsset.Succeeded())
    {
        SplineMesh->SetStaticMesh(MeshAsset.Object);
        SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, StartTangent);
        SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Detection
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
}

void ASeaweedInteractiveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickRotation(DeltaTime);
    TickSplineBend(DeltaTime);
}

void ASeaweedInteractiveActor::TickRotation(float DeltaTime)
{
    float Target = bShouldBend ? 1.f : 0.f;
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, Target, DeltaTime, BendSpeed);

    FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, CurrentAlpha);
    NewRotation.Roll = 0.f;
    SeaweedMesh->SetRelativeRotation(NewRotation);
}

void ASeaweedInteractiveActor::TickSplineBend(float DeltaTime)
{
    float Target = bShouldBend ? 1.f : 0.f;
    LerpAlpha = FMath::FInterpTo(LerpAlpha, Target, DeltaTime, BendSpeed);

    FVector CurEnd = FMath::Lerp(EndPos, EndPos + FVector(BendAmount, 0.f, 0.f), LerpAlpha);
    FVector CurTangent = FMath::Lerp(StartTangent, FVector(BendAmount * 0.5f, 0.f, 100.f), LerpAlpha);

    SplineMesh->SetStartAndEnd(StartPos, StartTangent, CurEnd, CurTangent);
}

void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (OtherActor == Player)
    {
        FVector SeaweedLocation = GetActorLocation();
        FVector PlayerLocation = Player->GetActorLocation();
        FVector ToPlayer = PlayerLocation - SeaweedLocation;
        ToPlayer.Normalize();

        FVector UpVector = SeaweedMesh->GetUpVector();
        FVector RotationAxis = FVector::CrossProduct(UpVector, ToPlayer).GetSafeNormal();
        float Dot = FVector::DotProduct(UpVector, ToPlayer);
        float Sign = FMath::Sign(Dot);

        FQuat QuatRot = FQuat(RotationAxis, FMath::DegreesToRadians(BendPitch * Sign));
        FRotator BentRot = (QuatRot * StartRotation.Quaternion()).Rotator();

        TargetRotation = BentRot;
        bShouldBend = true;
    }
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (OtherActor == Player)
    {
        bShouldBend = false;
    }
}
