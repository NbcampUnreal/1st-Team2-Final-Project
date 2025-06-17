#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "ADDroneSeller.generated.h"

class USoundSubsystem;
class UMissionSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentMoneyChangedDelegate, int32/*Changed Money*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetMoneyChangedDelegate, int32/*Changed Money*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSellOreDelegate, uint8 /*OreId*/, int32 /*OreMass*/);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADDroneSeller : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADDroneSeller();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

#pragma region Method
public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return bIsActive; }
	UFUNCTION()
	void DisableSelling();

	void Activate();

	UFUNCTION()
	void OnRep_IsActive();

	UFUNCTION()
	void OnRep_CurrentMoney();

	UFUNCTION()
	void OnRep_TargetMoney();

	FOnCurrentMoneyChangedDelegate OnCurrentMoneyChangedDelegate;
	FOnTargetMoneyChangedDelegate OnTargetMoneyChangedDelegate;
	FOnSellOreDelegate OnSellOreDelegate;

	void SetLightColor(FLinearColor NewColor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TemporarilyHighlightGreen(bool bReachedGoal);

protected:
	int32 SellAllExchangeableItems(AActor* InstigatorActor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, EditAnywhere)
	uint8 bIsActive : 1;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMoney)
	int32 CurrentMoney = 0;
	UPROPERTY(ReplicatedUsing = OnRep_TargetMoney)
	int32 TargetMoney = 1000;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class AADDrone> CurrentDrone = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UADInteractableComponent> InteractableComp;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;
	UPROPERTY()
	TObjectPtr<UMissionSubsystem> MissionSubsystem;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> RedMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> GreenMaterial = nullptr;

private:

#pragma endregion

#pragma region Getter, Setteer
public:
	int32 GetCurrentMoney() const { return CurrentMoney; }
	int32 GetTargetMoney() const { return TargetMoney; }

	void SetTargetMoney(int32 NewTargetMoney) 
	{ 
		TargetMoney = NewTargetMoney; 
		OnRep_TargetMoney();
	}

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const override;
	virtual FString GetInteractionDescription() const override;

private:

	void SetCurrentMoeny(const int32& NewCurrentMoney)
	{
		if (HasAuthority() == false)
		{
			return;
		}

		CurrentMoney = NewCurrentMoney;
		OnRep_CurrentMoney();
	}
	
	USoundSubsystem* GetSoundSubsystem();
	UMissionSubsystem* GetMissionSubsystem();
#pragma endregion

};
