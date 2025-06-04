#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "DataRow/PhaseGoalRow.h"
#include "LevelSelector.generated.h"

class ULevelSelectWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ALevelSelector : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	ALevelSelector();

protected:

	virtual void BeginPlay() override;

#pragma region Methods

public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void HandleMapChosen(EMapName InLevelID);

#pragma endregion

#pragma region Variables
private:
	UPROPERTY(EditDefaultsOnly, Category = "LevelSelectWidget")
	TSubclassOf<ULevelSelectWidget> LevelSelectWidgetClass;

	UPROPERTY()
	TObjectPtr<ULevelSelectWidget> LevelSelectWidget;

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

	uint8 bSelectLevel : 1 = false;
	EMapName LevelID;
#pragma endregion



#pragma region Getters / Setters
public:

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;

	EMapName GetLevelID() const { return LevelID; }
	bool IsSelectedLevel() const { return bSelectLevel; }

#pragma endregion

};
