#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CurrentZone.generated.h"

class AUnderwaterCharacter;
class UArrowComponent;

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

    UFUNCTION()
    void OnDeepTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDeepTriggerOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyCurrentForce();

public:

    // ShouldUseArrowDirection 옵션을 사용하지 않아야 적용된다.
    UPROPERTY(EditAnywhere, Category = "Current")
    FVector PushDirection = FVector(0.f, -1.f, 0.f);

    UPROPERTY(EditAnywhere, Category = "Current")
    float FlowStrength = 50.f;

    // 이 옵션을 사용하면 Arrow Component의 방향을 이용하여 PushDirection이 결정된다. 
    UPROPERTY(EditAnywhere, Category = "Current")
    uint8 bShouldUseArrowDirection : 1 = false;

protected:

    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> TriggerZone;

    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> DeepTriggerZone;

    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UArrowComponent> ArrowComponent;


private:
    // bool : 깊은 급류 안에 들어왔는지 확인용
    TMap<TObjectPtr<AUnderwaterCharacter>, bool> AffectedCharacters;
    FTimerHandle CurrentForceTimer;
    TMap<TObjectPtr<AUnderwaterCharacter>, float> OriginalSpeeds;
    TMap<TObjectPtr<AUnderwaterCharacter>, float> OriginalAccelerations;
};
