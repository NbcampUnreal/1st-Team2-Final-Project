// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LanternComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API ULanternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULanternComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Method

public:
	/** Lantern Toggle 요청 */
	void RequestToggleLanternLight();
	
protected:

	UFUNCTION()
	void OnRep_bIsLanternOn();
	
	void SpawnLight();

	/** Request Toggle Lantern Light를 서버에서 처리한다. */
	UFUNCTION(Server, Reliable)
	void S_ToggleLanternLight();
	void S_ToggleLanternLight_Implementation();

#pragma endregion

#pragma region Variable

private:
	
	/** 현재 라이트 활성화 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsLanternOn, Category = "Character|Lantern", meta = (AllowPrivateAccess = "true"))
	uint8 bIsLanternOn : 1;

	UPROPERTY()
	TObjectPtr<class USpotLightComponent> LanternLightComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Lantern")
	float LanternForwardOffset;

#pragma endregion

#pragma region Getter Setter

#pragma endregion
};
