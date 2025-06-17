#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADInteractableComponent.generated.h"

#define LOGIC(Verbosity, Format, ...) UE_LOG(ItemComponentLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(ItemComponentLog, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UADInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UADInteractableComponent();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	// ¿Ü°û¼± ¼³Á¤
	UFUNCTION(BlueprintCallable, Category = "Outline")
	void SetHighLight(bool bEnable);
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void Interact(AActor* InstigatorActor);

protected:


private:


#pragma endregion

#pragma region Variable
public:


protected:

	UPROPERTY()
	TArray<TObjectPtr<UMeshComponent>> MeshComponents;
	
private:
	int32 CustomStencilValue = 1;
	uint8 bHighlighted : 1 = false;
	uint8 bCanInteractable : 1 = true;

#pragma endregion

#pragma region Getter, Setteer
public:
	bool IsHighlighted() const { return bHighlighted; }
	bool CanInteractable() const { return bCanInteractable; }

	void SetInteractable(bool InCanInteractable) { bCanInteractable = InCanInteractable; }

#pragma endregion
	

		
};
