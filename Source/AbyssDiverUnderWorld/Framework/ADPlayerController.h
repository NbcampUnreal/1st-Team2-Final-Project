#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "ADPlayerController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerController : public APlayerController
{
	GENERATED_BODY()

	AADPlayerController();
	
protected:
	virtual void BeginPlay() override;

#pragma region Method
	UFUNCTION(Server, Reliable)
	void S_SetPlayerInfo(const FUniqueNetIdRepl& Id, const FString& Nickname);
	void S_SetPlayerInfo_Implementation(const FUniqueNetIdRepl& Id, const FString& Nickname);


	virtual void SetupInputComponent() override;
	void ToggleInventoryShowed(const FInputActionValue& InputActionValue);
	
#pragma endregion
	
#pragma region Variable

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPlayerHUDComponent> PlayerHUDComponent;
	
#pragma endregion 
};
