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
}

void ASeaweedInteractiveActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional
}

void ASeaweedInteractiveActor::ApplyPlayerProximityTorque()
{
    if (!PlayerActor || !SeaweedMesh) return;

    FVector PlayerLocation = PlayerActor->GetActorLocation();

    for (int32 i = 0; i < AllStemBoneNames.Num(); ++i)
    {
        const FName& BoneName = AllStemBoneNames[i];
        FVector BoneLocation = SeaweedMesh->GetSocketLocation(BoneName);
        float Distance = FVector::Dist(BoneLocation, PlayerLocation);

        float Influence = FMath::Clamp(1.f - (Distance / 300.f), 0.f, 1.f);
        float Multiplier = 1.f - (i / static_cast<float>(AllStemBoneNames.Num()));

        if (Influence > 0.f)
        {
            FVector ToPlayer = (BoneLocation - PlayerLocation).GetSafeNormal(); // ← 밀어내는 방향
            FVector Force = ToPlayer * Influence * Multiplier * 10000.f;

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

    // Constraint 하나당 한 번만 적용
    for (UPhysicsConstraintTemplate* Template : Asset->ConstraintSetup)
    {
        if (!Template) continue;

        FConstraintInstance& Inst = Template->DefaultInstance;

        // 현재 Constraint에 연결된 두 Bone 중 하나라도 해초 Stem에 포함돼야 함
        int32 Index = AllStemBoneNames.IndexOfByKey(Inst.ConstraintBone1);
        if (Index == INDEX_NONE)
        {
            Index = AllStemBoneNames.IndexOfByKey(Inst.ConstraintBone2);
            if (Index == INDEX_NONE) continue;
        }

        float Blend = static_cast<float>(Index) / AllStemBoneNames.Num();
        float Stiffness = FMath::Lerp(1000.f, 500.f, Blend);   // 아래쪽은 강하게
        float Damping = FMath::Lerp(150.f, 80.f, Blend);     // 감쇠력도 아래쪽 강하게
        float SwingLimit = FMath::Lerp(18.f, 12.f, Blend);     // 지나친 휘어짐 방지


        // SLERP 드라이브 적용
        Inst.SetAngularDriveMode(EAngularDriveMode::SLERP);
        Inst.ProfileInstance.AngularDrive.SlerpDrive.bEnablePositionDrive = true;
        Inst.ProfileInstance.AngularDrive.SlerpDrive.bEnableVelocityDrive = false;
        Inst.ProfileInstance.AngularDrive.SlerpDrive.Stiffness = Stiffness;
        Inst.ProfileInstance.AngularDrive.SlerpDrive.Damping = Damping;

        // 각도 제한
        Inst.ProfileInstance.ConeLimit.Swing1Motion = ACM_Limited;
        Inst.ProfileInstance.ConeLimit.Swing2Motion = ACM_Limited;
        Inst.ProfileInstance.ConeLimit.Swing1LimitDegrees = SwingLimit;
        Inst.ProfileInstance.ConeLimit.Swing2LimitDegrees = SwingLimit;
    }

    SeaweedMesh->SetEnableGravity(false); // 중력 OFF
    SeaweedMesh->RecreatePhysicsState();  // 물리 적용
}
