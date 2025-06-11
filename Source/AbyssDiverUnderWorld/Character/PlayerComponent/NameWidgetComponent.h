// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NameWidgetComponent.generated.h"

UENUM(BlueprintType)
enum class EBillboardRotationMode : uint8
{
	LookAtCamera, // Direction = Component - Camera
	ReverseCameraForward, // Direction = -(Camera Forward)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UNameWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UNameWidgetComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Method

protected:
	/** 로컬 플레이어의 카메라를 바라보도록 Billboard를 회전 */
	void UpdateBillboardRotation();

	/** 로컬 플레이어와의 거리를 확인해서 UI 가시성 여부 계산 */
	void UpdateDistanceVisibility();

	/** 현재 위젯 가시성 설정 */
	void SetVisibility(bool bNewVisibility);

#pragma endregion

#pragma region Variable

private:

	/** 이름 위젯에 표시될 텍스트 */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FString NameText;

	/** 이름이 표기되는 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|NameWidget")
	float DisplayDistance;

	/** 이름이 표기되고 사라질 떄까지의 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|NameWidget")
	float MaxDisplayDistance;

	/** Client에서 플레이하는 Local PlayerController */
	UPROPERTY()
	TObjectPtr<APlayerController> LocalPlayerController;

	/** 현재 위젯 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|NameWidget", meta = (AllowPrivateAccess = "true"))
	uint8 bIsEnabled : 1;
	
	/** 가시성 조건을 만족해서 Widget이 표기되는 지 여부 */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 bIsVisible : 1;

	/** 빌보드 회전 모드 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|NameWidget", meta = (AllowPrivateAccess = "true"))
	EBillboardRotationMode BillboardRotationMode;
	
#pragma endregion

public:

	/** 이름 위젯의 텍스트를 설정 */
	void SetNameText(const FString& NewName);

	/** 위젯 활성화 여부 설정 */
	UFUNCTION(BlueprintCallable)
	void SetEnable(bool bNewEnabled);
};
