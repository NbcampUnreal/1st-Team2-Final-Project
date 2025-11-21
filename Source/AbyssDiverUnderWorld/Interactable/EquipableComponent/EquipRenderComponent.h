#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipRenderComponent.generated.h"

class UEquipableComponent;
class UMeshComponent;
class USkeletalMesh;
class UStaticMesh;

USTRUCT()
struct FRenderEntry
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UMeshComponent> Mesh1P;

    UPROPERTY()
    TWeakObjectPtr<UMeshComponent> Mesh3P;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UEquipRenderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipRenderComponent();

#pragma region Method
public:
    UFUNCTION(BlueprintCallable)
    void AttachItem(AActor* RawItem, FName SocketName);

    UFUNCTION(BlueprintCallable)
    void DetachItem(AActor* RawItem);

    UEquipableComponent* FindEquipableComponent(AActor* RawItem);
    UMeshComponent* Create1PComponent(UEquipableComponent* EComp, ACharacter* OwningChar, FName SocketName);
    UMeshComponent* Create3PComponent(UEquipableComponent* EComp, ACharacter* OwningChar, FName SocketName);

	void SetRenderFirstPerson(bool bNewFirstPerson);
	
#pragma endregion

#pragma region Variable
private:
    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, FRenderEntry> ActiveEntries;
#pragma endregion

#pragma region Getter, Setter
		
};
