#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "ADPlayerController.generated.h"

enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerController : public APlayerController
{
	GENERATED_BODY()

	AADPlayerController();
	
protected:

	virtual void BeginPlay() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void PostNetInit() override;
	virtual void PostSeamlessTravel() override;

#pragma region Method

public:

	UFUNCTION(Client, Reliable)
	void C_OnPreClientTravel();
	void C_OnPreClientTravel_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPreClientTravel();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPostNetInit();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPostSeamlessTravel();



protected:

	UFUNCTION(Server, Reliable)
	void S_SetPlayerInfo(const FUniqueNetIdRepl& Id, const FString& Nickname);
	void S_SetPlayerInfo_Implementation(const FUniqueNetIdRepl& Id, const FString& Nickname);

	UFUNCTION(Server, Reliable)
	void S_RequestSelectLevel(const EMapName InLevelName);
	void S_RequestSelectLevel_Implementation(const EMapName InLevelName);

	UFUNCTION(Server, Reliable)
	void S_RequestStartGame();
	void S_RequestStartGame_Implementation();

	virtual void SetupInputComponent() override;
	void ShowInventory(const FInputActionValue& InputActionValue);
	void HideInventory(const FInputActionValue& InputActionValue);

	UFUNCTION(Exec)
	void ToggleTestHUD();
	

#pragma endregion
	
#pragma region Variable

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPlayerHUDComponent> PlayerHUDComponent;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UInteractionDescriptionWidget> InteractionWidgetClass;

	UPROPERTY()
	TObjectPtr<UInteractionDescriptionWidget> InteractionWidget;
	
#pragma endregion 

#pragma region Getters / Setters

public:

	UPlayerHUDComponent* GetPlayerHUDComponent() const { return PlayerHUDComponent; }

#pragma endregion

};
