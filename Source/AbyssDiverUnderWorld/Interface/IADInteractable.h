#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "IADInteractable.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Pickup			UMETA(DisplayName = "줍기"),
	Mining			UMETA(DisplayName = "채광하기"),
	GiveOre			UMETA(DisplayName = "광석 제출하기"),
	SendDrone		UMETA(DisplayName = "드론 보내기"),
	OpenShop		UMETA(DisplayName = "상점 열기"),
	GoToSubmarine	UMETA(DisplayName = "잠수정으로 이동하기"),
	ReviveCharacter UMETA(DisplayName = "살리기"),
	GoToNextLevel	UMETA(DisplayName = "다음 맵으로 이동하기")
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIADInteractable : public UInterface
{
	GENERATED_BODY()
};

class ABYSSDIVERUNDERWORLD_API IIADInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* InstigatorActor);
	virtual void Interact_Implementation(AActor* InstigatorActor);
	// Ȧ�� ���� �� ȣ���� �Լ�
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void InteractHold(AActor* InstigatorActor);
	virtual void InteractHold_Implementation(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnHoldStart(APawn* InstigatorPawn);
	virtual void OnHoldStart_Implementation(APawn* InstigatorPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnHoldStop(APawn* InstigatorPawn);
	virtual void OnHoldStop_Implementation(APawn* InstigatorPawn);

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanHighlight() const;
	virtual bool CanHighlight_Implementation() const { return true; }
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	float GetHoldDuration(AActor* InstigatorActor) const;
	virtual float GetHoldDuration_Implementation(AActor* InstigatorActor) const;

	virtual UADInteractableComponent* GetInteractableComponent() const = 0;
	virtual bool IsHoldMode() const = 0;
	virtual FString GetInteractionDescription() const;
};
