#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SeaweedInteractiveActor.generated.h"

class USphereComponent;
class USkeletalMeshComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaweedInteractiveActor : public AActor
{
    GENERATED_BODY()

public:
    ASeaweedInteractiveActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // 오버랩 감지
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 충돌 감지
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    // 해초 휘어짐 및 회복 처리
    void ApplyPlayerProximityTorque();
    void UpdatePhysicsBlendWeight();
    void ConfigureAngularDrives();
protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkeletalMeshComponent> SeaweedMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> DetectionSphere;

    UPROPERTY(EditAnywhere, Category = "Seaweed")
    TArray<FName> AllStemBoneNames;

    UPROPERTY()
    TObjectPtr<AActor> PlayerActor;

    FVector LastHitLocation;
};
