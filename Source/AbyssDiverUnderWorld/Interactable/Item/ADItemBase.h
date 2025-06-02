#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Container/FStructContainer.h"
#include "ADItemBase.generated.h"

#define LOGI(Verbosity, Format, ...) UE_LOG(ItemLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(ItemLog, Log, All);

class UADInteractableComponent;
class UProjectileMovementComponent;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADItemBase : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	AADItemBase();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:	

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	virtual void HandlePickup(APawn* InstigatorPawn);

	

protected:
	UFUNCTION()
	void OnRep_ItemData();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	

#pragma endregion

#pragma region Variable
public:
	// 드롭 모션 적용을 위한 발사체 컴포넌트
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> DropMovement;

	UPROPERTY(ReplicatedUsing = OnRep_ItemData, EditAnywhere, Category = "Item")
	FItemData ItemData;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UADInteractableComponent> InteractableComp;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

	// TODO : 인벤토리 컴포넌트 참조
	// TODO : PickupSound 등 획득 시 효과 추가
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;

	float WaterGravityScale = 0.3f;
	float WaterDampingFactor = 1.f;

#pragma endregion

#pragma region Getter, Setteer
public:
	void SetItemMass(int32 InMass);
	void SetPrice(int32 InPrice);
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;
#pragma endregion

};
