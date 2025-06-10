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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

    UFUNCTION()
    void OnRep_BendState();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seaweed", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMesh> SeaweedMeshAsset;

    // Runtime
    FRotator StartRotation;

    UPROPERTY(ReplicatedUsing = OnRep_BendState)
    FRotator TargetRotation;

    float CurrentAlpha = 0.0f;
    float LerpAlpha = 0.0f;

    UPROPERTY(ReplicatedUsing = OnRep_BendState)
    bool bShouldBend = false;

    int32 OverlappingCharacterCount = 0;

    FVector StartPos = FVector::ZeroVector;
    FVector EndPos = FVector(0.f, 0.f, 795.f);
    FVector StartTangent = FVector(0.f, 0.f, 100.f);

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendPitch = -26.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seaweed")
    float BendAmount = 30.f;
#pragma endregion
};