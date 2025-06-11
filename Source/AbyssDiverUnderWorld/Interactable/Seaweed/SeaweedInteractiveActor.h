#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PoseableMeshComponent.h"
#include "SeaweedInteractiveActor.generated.h"

class USphereComponent;
class USceneComponent;
class UPoseableMeshComponent;

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

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnRep_BendState();

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPoseableMeshComponent> SeaweedMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> DetectionSphere;

    TArray<FName> BendingBoneNames = {
        "Bone_004", "Bone_005", "Bone_006", "Bone_007" // 하단 본 제외
    };

    TMap<FName, FQuat> OriginalBoneRotations;

    UPROPERTY(ReplicatedUsing = OnRep_BendState)
    uint8 bShouldBend : 1;

    int32 OverlappingCharacterCount = 0;
    float CurrentAlpha = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Seaweed")
    float BendSpeed = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Seaweed")
    float BendAmount = 35.f;

    UPROPERTY(Replicated)
    FVector BendAxis = FVector::RightVector;
};
