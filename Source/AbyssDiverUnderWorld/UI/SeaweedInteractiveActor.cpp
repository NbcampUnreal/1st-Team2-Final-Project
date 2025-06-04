#include "SeaweedInteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ASeaweedInteractiveActor::ASeaweedInteractiveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SeaweedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeaweedMesh"));
    RootComponent = SeaweedMesh;

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
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

    FRotator Current = SeaweedMesh->GetRelativeRotation();
    FRotator Target = bShouldBend ? TargetRotation : StartRotation;

    FRotator NewRotation = FMath::RInterpTo(Current, Target, DeltaTime, BendSpeed);
    NewRotation.Roll = 0.f; // 혹시 모르게 다시 보정
    NewRotation.Yaw = StartRotation.Yaw;

    SeaweedMesh->SetRelativeRotation(NewRotation);
}



void ASeaweedInteractiveActor::TickRotation(float DeltaTime)
{
    float Target = bShouldBend ? 1.f : 0.f;
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, Target, DeltaTime, BendSpeed);

    FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, CurrentAlpha);
    NewRotation.Roll = 0.f; // ✅ 혹시라도 회전 틀어지지 않도록 보정
    SeaweedMesh->SetRelativeRotation(NewRotation);
}

void ASeaweedInteractiveActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (OtherActor == Player)
    {
        // 회전 방향 신경 안 쓰고 그냥 뒤로 숙이기만
        TargetRotation = StartRotation + FRotator(BendPitch, 0.f, 0.f);
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
