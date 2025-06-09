// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NameWidgetComponent.generated.h"


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
	/** 로컬 플레이어의 카메라를 바라보도록 Billboard를 회전시킵니다. */
	void UpdateBillboardRotation();

	/** 로컬 플레이어와의 거리를 확인해서 이름을 표시할지 여부를 결정합니다. */
	void UpdateDistanceVisibility();
	
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

	/** 현재 이름 위젯이 표시되고 있는지 여부 */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 bIsVisible : 1;
	
#pragma endregion

public:

	/** 이름 위젯의 텍스트를 설정 */
	void SetNameText(const FString& NewName);

	/** 현재 위젯 가시성 설정 */
	void SetVisibility(bool bNewVisibility);

};
