#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SeaweedInteractiveActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class USplineMeshComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaweedInteractiveActor : public AActor
{
    GENERATED_BODY()

#pragma region Constructor
public:
    ASeaweedInteractiveActor();
#pragma endregion

#pragma region Lifecycle
protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region Method
protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void TickRotation(float DeltaTime);
    void TickSplineBend(float DeltaTime);
#pragma endregion

#pragma region Variable
protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> SeaweedMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USplineMeshComponent> SplineMesh;

    // Runtime
    FRotator StartRotation;
    FRotator TargetRotation;

    float CurrentAlpha = 0.0f;
    float LerpAlpha = 0.0f;
    uint8 bShouldBend : 1;

    FVector StartPos = FVector::ZeroVector;
    FVector EndPos = FVector(0.f, 0.f, 795.f);
    FVector StartTangent = FVector(0.f, 0.f, 100.f);

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendSpeed = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendPitch = -30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendAmount = 100.f;
#pragma endregion
};
