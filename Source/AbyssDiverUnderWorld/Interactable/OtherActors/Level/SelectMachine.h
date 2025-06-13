// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SelectMachine.generated.h"

enum class EMapName : uint8;
class UWidgetComponent;
	
UCLASS()
class ABYSSDIVERUNDERWORLD_API ASelectMachine : public AActor
{
	GENERATED_BODY()
	
public:	
	ASelectMachine();

#pragma region Methods
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void UpdatelevelImage();
	void UpdatelevelImage_Implementation();

	void HandlePrevLevel(AActor* InteractInstigator);
	void HandleNextLevel(AActor* InteractInstigator);
	void HandleSelectLevel(AActor* InteractInstigator);
	void HandleTravelLevel(AActor* InteractInstigator);

	bool IsConditionMet();
#pragma endregion

#pragma region Variables

protected:
	//UPROPERTY(EditAnywhere)
	//TArray < TObjectPtr<class AInteractableButton>> Buttons;
	int8 CurrentLevelIndex = 0;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> SelectMissionWidgetComp;

	TArray<EMapName> LevelIDs;

	uint8 bSelectLevel : 1 = false;
	EMapName LevelID;
#pragma endregion

#pragma region Getter/Setter
public:
	UFUNCTION(BlueprintCallable)
	uint8 GetCurrentLevelIndex() { return CurrentLevelIndex; }
#pragma endregion
};
