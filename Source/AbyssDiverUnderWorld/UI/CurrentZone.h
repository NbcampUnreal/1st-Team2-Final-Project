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
    // 생성자
    ACurrentZone();

protected:
    // BeginPlay 등 생명주기 함수 (필요 시 사용)

#pragma region Method
public:
    // Tick
    virtual void Tick(float DeltaTime) override;

protected:
    // 오버랩 처리 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion

#pragma region Variable
public:
    // 밀어내는 힘의 방향과 크기
    UPROPERTY(EditAnywhere, Category = "Current")
    FVector PushDirection = FVector(1.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, Category = "Current")
    float PushForce = 1000000.f;

protected:
    // 컴포넌트: 박스 트리거
    UPROPERTY(VisibleAnywhere, Category = "Component")
    UBoxComponent* TriggerZone;

private:
    // 영향을 받는 캐릭터 목록
    TSet<ACharacter*> AffectedCharacters;
#pragma endregion
};
