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

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /** 플레이어와의 거리 기반으로 Torque 적용 */
    void ApplyPlayerProximityTorque();

    /** 플레이어 거리 기반으로 AnimBP의 PhysicsBlendAlpha 업데이트 */
    void UpdatePhysicsBlendWeight();

    UFUNCTION()
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

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    FVector LastHitLocation;


};
