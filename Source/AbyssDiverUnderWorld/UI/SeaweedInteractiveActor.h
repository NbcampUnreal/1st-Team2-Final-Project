#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SeaweedInteractiveActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaweedInteractiveActor : public AActor
{
    GENERATED_BODY()

public:
    ASeaweedInteractiveActor();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void TickRotation(float DeltaTime);

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SeaweedMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    // Runtime State
    FRotator StartRotation;
    FRotator TargetRotation;

    float CurrentAlpha = 0.0f;
    bool bShouldBend = false;

    // Config
    UPROPERTY(EditAnywhere, Category = "Seaweed")
    float BendSpeed = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Seaweed")
    float BendPitch = -30.f;
};
