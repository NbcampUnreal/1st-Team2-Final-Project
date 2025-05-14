// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHUDComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UPlayerHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerHUDComponent();

protected:
	UFUNCTION()
	// Called when the game starts
	virtual void BeginPlay() override;

#pragma region Method

public:
	
	void SetVisibility(bool NewVisible) const;

private:

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
#pragma endregion

#pragma region Variable

private:
	
	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPlayerHUDWidget> HudWidgetClass;

	UPROPERTY()
	TObjectPtr<class UPlayerHUDWidget> HudWidget;

	
#pragma endregion
};
