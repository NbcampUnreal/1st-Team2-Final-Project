#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "SeaMonster.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaMonster : public AADItemBase
{
	GENERATED_BODY()
	
	
public:
	ASeaMonster();

protected:

#pragma region Method
public:
	// Interact 시에 호출되는 함수
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void MorphInto();

protected:


private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

private:

#pragma endregion

#pragma region Getter, Setteer
public:


#pragma endregion

};
