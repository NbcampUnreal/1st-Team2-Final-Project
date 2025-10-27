#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainMenuGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivateButtons, bool, bIsActivate);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AMainMenuGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

#pragma region methods
private:
	UFUNCTION(Exec, Category = "Cheat")
	void ActivateButtons();

#pragma endregion
#pragma region properties
public:
	UPROPERTY(BlueprintAssignable)
	FOnActivateButtons OnActivateButtonsDelegate;


#pragma endregion



};
