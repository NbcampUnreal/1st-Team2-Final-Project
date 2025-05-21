#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SpikeHazard.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASpikeHazard : public AActor
{
    GENERATED_BODY()

public:
    // 생성자
    ASpikeHazard();

protected:
    // 생명주기 함수
    virtual void BeginPlay() override;

#pragma region Method
public:
    // 오버랩 이벤트 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion

#pragma region Variable
public:
    // 데미지 및 넉백 세기
    UPROPERTY(EditAnywhere, Category = "Hazard")
    float DamageAmount = 20.f;

    UPROPERTY(EditAnywhere, Category = "Hazard")
    float KnockbackStrength = 600.f;

protected:
    // 컴포넌트 (TObjectPtr로 관리)
    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> CollisionBox;

    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UStaticMeshComponent> SpikeMesh;
#pragma endregion
};
