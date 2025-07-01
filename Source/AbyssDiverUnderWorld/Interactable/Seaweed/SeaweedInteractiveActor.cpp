// SeaweedInteractiveActor.cpp
#include "SeaweedInteractiveActor.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "SeaweedAnimInstance.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    SeaweedMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SeaweedMesh"));
    SeaweedMesh->SetupAttachment(RootComponent);
    SeaweedMesh->SetSimulatePhysics(true);
    SeaweedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SeaweedMesh->SetGenerateOverlapEvents(false);

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(300.f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    AllStemBoneNames = {
        "Bone_000", "Bone_001", "Bone_002", "Bone_003",
        "Bone_004", "Bone_005", "Bone_006", "Bone_007"
    };
}

void ASeaweedInteractiveActor::BeginPlay()
{
    Super::BeginPlay();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapBegin);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ASeaweedInteractiveActor::OnOverlapEnd);
    SeaweedMesh->OnComponentHit.AddDynamic(this, &ASeaweedInteractiveActor::OnHit);

    PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    ConfigureAngularDrives();
    SeaweedMesh->SetAllBodiesBelowSimulatePhysics("Bone_000", true, false);
    SeaweedMesh->SetEnableGravity(false);
    SeaweedMesh->SetConstraintMode(EDOFMode::SixDOF);
    SeaweedMesh->WakeAllRigidBodies();
    SeaweedMesh->SetPhysicsBlendWeight(1.0f);
}

void ASeaweedInteractiveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyPlayerProximityTorque();
    UpdatePhysicsBlendWeight();
}

void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    LastHitLocation = SweepResult.ImpactPoint;
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ASeaweedInteractiveActor::ApplyPlayerProximityTorque()
{
    if (!SeaweedMesh || LastHitLocation.IsZero()) return;

    for (int32 i = 0; i < AllStemBoneNames.Num(); ++i)
    {
        const FName& BoneName = AllStemBoneNames[i];
        FVector BoneLocation = SeaweedMesh->GetSocketLocation(BoneName);
        float Distance = FVector::Dist(BoneLocation, LastHitLocation);

        if (Distance > 120.f) continue;

        float Influence = 1.f - (Distance / 120.f);
        float Strength = FMath::Pow(Influence, 2.f);

        if (Strength > 0.f)
        {
            FVector Direction = (BoneLocation - LastHitLocation).GetSafeNormal();
            Direction.Z = 0.25f;
            Direction.Normalize();

            FVector Force = Direction * Strength * 18000.f;
            SeaweedMesh->AddForce(Force, BoneName, true);
        }
    }
}

void ASeaweedInteractiveActor::UpdatePhysicsBlendWeight()
{
    if (!SeaweedMesh || !PlayerActor) return;

    float Distance = FVector::Dist(SeaweedMesh->GetComponentLocation(), PlayerActor->GetActorLocation());
    float Alpha = FMath::Clamp((Distance - 50.f) / (300.f - 50.f), 0.f, 1.f);

    USeaweedAnimInstance* AnimInstance = Cast<USeaweedAnimInstance>(SeaweedMesh->GetAnimInstance());
    if (AnimInstance)
    {
        AnimInstance->SetPhysicsBlendAlpha(1.0f);
    }
}

void ASeaweedInteractiveActor::ConfigureAngularDrives()
{
    if (!SeaweedMesh || !SeaweedMesh->GetPhysicsAsset()) return;

    UPhysicsAsset* Asset = SeaweedMesh->GetPhysicsAsset();

    for (int32 i = 0; i < AllStemBoneNames.Num(); ++i)
    {
        const FName& BoneName = AllStemBoneNames[i];
        float Blend = static_cast<float>(i) / (AllStemBoneNames.Num() - 1);

        float Stiffness = FMath::Lerp(5000000.f, 2000000.f, Blend);
        float Damping = FMath::Lerp(60000.f, 30000.f, Blend);
        float MaxForce = 1e9f;

        FConstraintInstance* ConstraintInst = SeaweedMesh->FindConstraintInstance(BoneName);
        if (!ConstraintInst) continue;

        ConstraintInst->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
        ConstraintInst->SetAngularDriveParams(Stiffness, Damping, MaxForce);
        ConstraintInst->SetAngularVelocityDriveTwistAndSwing(false, false);
        ConstraintInst->SetOrientationDriveTwistAndSwing(true, true);

        if (Blend > 0.7f)
        {
            ConstraintInst->SetAngularSwing1Limit(ACM_Limited, 45.f);
            ConstraintInst->SetAngularSwing2Limit(ACM_Limited, 45.f);
            ConstraintInst->SetAngularTwistLimit(ACM_Limited, 20.f);
        }
        else
        {
            ConstraintInst->SetAngularSwing1Limit(ACM_Limited, 25.f);
            ConstraintInst->SetAngularSwing2Limit(ACM_Limited, 25.f);
            ConstraintInst->SetAngularTwistLimit(ACM_Limited, 10.f);
        }

        ConstraintInst->ProfileInstance.ConeLimit.bSoftConstraint = true;
        ConstraintInst->ProfileInstance.TwistLimit.bSoftConstraint = true;
    }

    SeaweedMesh->RecreatePhysicsState();
    SeaweedMesh->WakeAllRigidBodies();
    SeaweedMesh->SetSimulatePhysics(false);
    SeaweedMesh->SetSimulatePhysics(true);
}

void ASeaweedInteractiveActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    LastHitLocation = Hit.ImpactPoint;

    UE_LOG(LogTemp, Warning, TEXT("[Seaweed] Hit at: %s"), *LastHitLocation.ToString());

    if (USeaweedAnimInstance* AnimInstance = Cast<USeaweedAnimInstance>(SeaweedMesh->GetAnimInstance()))
    {
        AnimInstance->SetPhysicsBlendAlpha(1.0f);
        AnimInstance->CurrentBlendTime = 0.0f;
    }
}