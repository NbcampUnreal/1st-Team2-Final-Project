#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystem.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;
class UUpgradeComponent;

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

	UPROPERTY(Replicated)
	int32 TotalPeronalCredit;


	UPROPERTY(Replicated)
	int32 DeathCount;

	UPROPERTY(Replicated)
	int32 SafeReturnCount;

	UPROPERTY(Replicated)
	int32 MonsterKillCount;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UADInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUpgradeComponent> UpgradeComp;

#pragma endregion

#pragma region Getter/Setter
public:

	UADInventoryComponent* GetInventory() { return InventoryComp; };

	const FString& GetNickname() const { return PlayerNickname; }

	FORCEINLINE UADInventoryComponent* GetInventory() const { return InventoryComp; };
	FORCEINLINE UUpgradeComponent* GetUpgradeComp() const { return UpgradeComp; };


#pragma endregion
};
