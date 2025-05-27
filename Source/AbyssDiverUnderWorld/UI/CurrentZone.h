#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CurrentZone.generated.h"

class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ACurrentZone : public AActor
{
    GENERATED_BODY()

public:
    ACurrentZone();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyCurrentForce();

public:
    UPROPERTY(EditAnywhere, Category = "Current")
    FVector PushDirection = FVector(0.f, -1.f, 0.f);

    UPROPERTY(EditAnywhere, Category = "Current")
    float PushForce = 0.35f;

    UPROPERTY(EditAnywhere, Category = "Current")
    float FlowStrength = 50.f;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> TriggerZone;

private:
    TSet<TObjectPtr<AUnderwaterCharacter>> AffectedCharacters;
    FTimerHandle CurrentForceTimer;
    TMap<TObjectPtr<AUnderwaterCharacter>, float> OriginalSpeeds;
    TMap<TObjectPtr<AUnderwaterCharacter>, float> OriginalAccelerations;
};
