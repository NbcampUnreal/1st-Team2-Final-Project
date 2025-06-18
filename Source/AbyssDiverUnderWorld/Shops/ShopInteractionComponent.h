#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ShopInteractionComponent.generated.h"

class AShop;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class ABYSSDIVERUNDERWORLD_API UShopInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShopInteractionComponent();

protected:

	virtual void BeginPlay() override;

#pragma region Methods

public:

	UFUNCTION(Client, Reliable)
	void C_OpenShop(AShop* InteractingShop);
	void C_OpenShop_Implementation(AShop* InteractingShop);

	UFUNCTION(Server, Reliable)
	void S_RequestBuyItem(uint8 ItemId, uint8 Quantity);
	void S_RequestBuyItem_Implementation(uint8 ItemId, uint8 Quantity);

	UFUNCTION(Server, Reliable)
	void S_RequestBuyItems(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList);
	void S_RequestBuyItems_Implementation(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList);

#pragma endregion


#pragma region Variables

private:

	UPROPERTY()
	TObjectPtr<AShop> CurrentInteractingShop;

#pragma endregion

#pragma region Getters, Setters
public:

	AShop* GetCurrentInteractingShop() const;
	void SetCurrentInteractingShop(AShop* NewInteractingShop);

#pragma endregion

};
