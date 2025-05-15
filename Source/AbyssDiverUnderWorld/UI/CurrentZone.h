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
#pragma region Method
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyCurrentForce(); // 💡 타이머에서 호출할 함수
#pragma endregion

#pragma region Variable
public:
    UPROPERTY(EditAnywhere, Category = "Current")
    FVector PushDirection = FVector(0.f, -1.f, 0.f); // 기본 방향

    UPROPERTY(EditAnywhere, Category = "Current")
    float PushForce = 100.f;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> TriggerZone;

private:
    TSet<AUnderwaterCharacter*> AffectedCharacters;

    FTimerHandle CurrentForceTimer;
    TMap<AUnderwaterCharacter*, float> OriginalSpeeds;
    TMap<AUnderwaterCharacter*, float> OriginalAccelerations;

#pragma endregion
};
