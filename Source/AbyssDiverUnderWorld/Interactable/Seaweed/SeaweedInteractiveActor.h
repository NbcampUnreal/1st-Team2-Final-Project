#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SeaweedInteractiveActor.generated.h"

class USplineMeshComponent;
class UStaticMeshComponent;
class USphereComponent;
class USceneComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaweedInteractiveActor : public AActor
{
    GENERATED_BODY()

public:
    ASeaweedInteractiveActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Overlap Events
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
#pragma endregion

#pragma region Tick Functions
protected:
    void TickRotation(float DeltaTime);
    void TickSplineBend(float DeltaTime);
#pragma endregion

#pragma region Components
protected:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere)
    USplineMeshComponent* SplineMesh;

    UPROPERTY(VisibleAnywhere)
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* SeaweedSkeletalMesh;
#pragma endregion

#pragma region Config
public:
    UPROPERTY(EditAnywhere)
    UStaticMesh* SeaweedMeshAsset;

    UPROPERTY(EditAnywhere)
    float BendSpeed = 1.0f;

    UPROPERTY(EditAnywhere)
    float RecoverSpeed = 0.5f;

    UPROPERTY(EditAnywhere)
    float BendPitch = -26.f;

    UPROPERTY(EditAnywhere)
    float BendAmount = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BendAlpha = 0.f;
#pragma endregion

#pragma region Runtime
protected:
    FRotator StartRotation;

    UPROPERTY(ReplicatedUsing = OnRep_BendState)
    FRotator TargetRotation;

    UPROPERTY(ReplicatedUsing = OnRep_BendState)
    bool bShouldBend = false;

    float CurrentAlpha = 0.f;
    float LerpAlpha = 0.f;

    int32 OverlappingCharacterCount = 0;
#pragma endregion
};
