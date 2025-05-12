#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ShopButton.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AShopButton : public AActor
{
	GENERATED_BODY()
	
public:	

	AShopButton();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
