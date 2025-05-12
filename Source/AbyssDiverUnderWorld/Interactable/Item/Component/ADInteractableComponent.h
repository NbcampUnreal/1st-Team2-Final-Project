#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADInteractableComponent.generated.h"

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

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion
	

		
};
