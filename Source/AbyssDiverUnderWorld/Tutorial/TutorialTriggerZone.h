// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialTriggerZone.generated.h"

class ATutorialManager;
class UBoxComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialTriggerZone : public AActor
{
    GENERATED_BODY()

    public:
    ATutorialTriggerZone();

protected:
    virtual void BeginPlay() override;

#pragma region Components

    UPROPERTY(VisibleAnywhere, Category = "Component")
    TObjectPtr<UBoxComponent> TriggerBox;

#pragma endregion

#pragma region Tutorial

    // 트리거가 겹쳐질 경우 튜토리얼 매니저에게 시작 명령
    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TObjectPtr<ATutorialManager> TutorialManager;

#pragma endregion

#pragma region Overlap

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

};
