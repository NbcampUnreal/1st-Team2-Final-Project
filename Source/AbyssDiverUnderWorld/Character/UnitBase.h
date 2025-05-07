// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnitBase.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnitBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUnitBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
#pragma region Method
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
#pragma endregion

#pragma region Variable

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stat")
	TObjectPtr<class UStatComponent> StatComponent;

#pragma endregion
};
