#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/IADInteractable.h"

#include "MissionSelector.generated.h"

class UMissionSelectWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMissionSelector : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	

	AMissionSelector();

protected:

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

#pragma region Methods

public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void OpenMissionSelectWidget();

#pragma endregion

#pragma region Variables
private:

	UPROPERTY(EditDefaultsOnly, Category = "MissionSelectorSettings")
	TSubclassOf<UMissionSelectWidget> MissionSelectWidgetClass;

	UPROPERTY()
	TObjectPtr<UMissionSelectWidget> MissionSelectWidget;

	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;

#pragma endregion



#pragma region Getters / Setters

public:

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;


#pragma endregion

};
