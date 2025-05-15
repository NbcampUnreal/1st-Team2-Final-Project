// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystem.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AADPlayerState();

protected:

	virtual void BeginPlay() override;
	virtual void PostNetInit() override;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	void SetPlayerInfo(const FUniqueNetIdRepl& InId, const FString& InNickname);

	UFUNCTION()
	void OnRep_Nickname();


#pragma region Variable
protected:
	UPROPERTY(Replicated)
	FUniqueNetIdRepl ADPlayerID;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Nickname)
	FString PlayerNickname;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UADInventoryComponent> InventoryComp;
#pragma endregion
	
#pragma region Getter/Setter
public:
	UADInventoryComponent* GetInventory() { return InventoryComp; };

	const FString& GetNickname() const { return PlayerNickname; }
	const FUniqueNetIdRepl& GetPlayerId() const { return ADPlayerID; }
#pragma endregion
};
