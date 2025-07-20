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
    // 수동으로 다음 단계 강제 트리거
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void TriggerNextStep();

    // 현재 단계 재생
    void PlayCurrentStep();

    // 다음 단계로 이동
    void AdvanceStep();
protected:

#pragma endregion

#pragma region Variable

protected:
    // 튜토리얼 단계들을 가진 데이터 테이블
    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TObjectPtr<UDataTable> mTutorialDataTable;

    // 현재 진행 중인 단계 인덱스
    int32 mCurrentStepIndex = 0;

    // 튜토리얼 단계들의 RowName 목록
    TArray<FName> mStepRowNames;

    // 자동 진행 시 사용할 타이머 핸들
    FTimerHandle mStepTimerHandle;

    // 위젯 클래스 참조
    UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
    TSubclassOf<class UTutorialSubtitle> mTutorialSubtitleClass;

    // 생성된 자막 위젯 인스턴스
    UPROPERTY()
    TObjectPtr<UTutorialSubtitle> mSubtitleWidget;
#pragma endregion
};
