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

    ConfigureAngularDrives(); // 🔥 물리 세팅 한번에 초기화
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
    // Optional
    LastHitLocation = SweepResult.ImpactPoint;
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional
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
            Direction.Z = 0.25f; // 살짝 위로 밀려나듯이
            Direction.Normalize();

            FVector Force = Direction * Strength * 18000.f;

            UE_LOG(LogTemp, Warning, TEXT("ApplyForce to %s | Strength: %.2f"), *BoneName.ToString(), Strength);
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
        AnimInstance->PhysicsBlendAlpha = Alpha;
    }
}

void ASeaweedInteractiveActor::ConfigureAngularDrives()
{
    if (!SeaweedMesh || !SeaweedMesh->GetPhysicsAsset()) return;

    UPhysicsAsset* Asset = SeaweedMesh->GetPhysicsAsset();

    for (int32 i = 0; i < AllStemBoneNames.Num(); ++i)
    {
        const FName& BoneName = AllStemBoneNames[i];

        float Blend = static_cast<float>(i) / AllStemBoneNames.Num();

        // 아래는 단단하게, 위는 유연하게
        float Stiffness = FMath::Lerp(3000.f, 500.f, Blend);   // 아래 = 3000, 위 = 500
        float Damping = FMath::Lerp(250.f, 80.f, Blend);     // 위로 갈수록 여유롭게 흔들림

        for (UPhysicsConstraintTemplate* Template : Asset->ConstraintSetup)
        {
            if (!Template || Template->DefaultInstance.JointName != BoneName) continue;

            FConstraintInstance& Constraint = Template->DefaultInstance;

            Constraint.SetAngularDriveMode(EAngularDriveMode::SLERP);
            Constraint.ProfileInstance.ConeLimit.bSoftConstraint = true;
            Constraint.ProfileInstance.TwistLimit.bSoftConstraint = true;

            Constraint.SetOrientationDriveSLERP(true);
            Constraint.SetAngularVelocityDriveSLERP(false);

            Constraint.SetAngularDriveParams(Stiffness, Damping, 100000.f); // MaxForce 꼭 높게
        }
    }

    SeaweedMesh->RecreatePhysicsState(); // 물리 적용
}


void ASeaweedInteractiveActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    LastHitLocation = Hit.ImpactPoint;

    UE_LOG(LogTemp, Warning, TEXT("[Seaweed] Hit at: %s"), *LastHitLocation.ToString());
}
