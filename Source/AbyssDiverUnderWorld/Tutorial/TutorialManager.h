// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialStepData.h"
#include "TutorialManager.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:
    ATutorialManager();

protected:
    virtual void BeginPlay() override;

#pragma region Method

public:
    // �������� ���� �ܰ� ���� Ʈ����
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void TriggerNextStep();

protected:
    // ���� �ܰ� ���
    void PlayCurrentStep();

    // ���� �ܰ�� �̵�
    void AdvanceStep();

#pragma endregion

#pragma region Variable

protected:
    // Ʃ�丮�� �ܰ���� ���� ������ ���̺�
    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TObjectPtr<UDataTable> mTutorialDataTable;

    // ���� ���� ���� �ܰ� �ε���
    int32 mCurrentStepIndex = 0;

    // Ʃ�丮�� �ܰ���� RowName ���
    TArray<FName> mStepRowNames;

    // �ڵ� ���� �� ����� Ÿ�̸� �ڵ�
    FTimerHandle mStepTimerHandle;

#pragma endregion
};
