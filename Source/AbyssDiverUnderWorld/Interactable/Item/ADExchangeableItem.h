﻿#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/ADItemBase.h"
#include "ADExchangeableItem.generated.h"

class UProjectileMovementComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADExchangeableItem : public AADItemBase
{
	GENERATED_BODY()
	
public:
	AADExchangeableItem();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION()
	void OnRep_TotalPrice();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void CalculateTotalPrice();

	virtual void HandlePickup(APawn* InstigatorPawn) override;


protected:


private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion

#pragma region Variable
public:
	// 미션 관련 델리게이트
	DECLARE_MULTICAST_DELEGATE_TwoParams(
		FOnItemPickedDelegate,
		uint8, 
		APawn*
	);
	FOnItemPickedDelegate OnItemPicked;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exchange")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exchange")
	int32 Mass = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exchange")
	int32 ValuePerUnit = 10;
	UPROPERTY(ReplicatedUsing = OnRep_TotalPrice, EditAnywhere, BlueprintReadWrite, Category = "Exchange")
	int32 TotalPrice = 0;

	


private:
	//float WaterGravityScale = 0.3f;
	//float WaterDampingFactor = 1.f;

#pragma endregion

#pragma region Getter, Setteer
public:
	int32 GetMass() const { return Mass; }
	int32 GetTotalPrice() const { return TotalPrice; }

#pragma endregion

};
