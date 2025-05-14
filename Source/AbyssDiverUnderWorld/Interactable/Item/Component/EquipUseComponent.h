#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DataRow/FADItemDataRow.h"
#include "EquipUseComponent.generated.h"

class AADProjectileBase;
class UUserWidget;

enum class EAction : uint8
{
	None,
	WeaponFire,
	WeaponReload,
	ToggleBoost,
	ToggleNVGToggle,
	ApplyChargeUI
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UEquipUseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipUseComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Method
public:
	// 서버 RPC
	UFUNCTION(Server, Reliable)
	void S_LeftClick();
	void S_LeftClick_Implementation();
	UFUNCTION(Server, Reliable)
	void S_RKey();
	void S_RKey_Implementation();
	UFUNCTION()
	void OnRep_Amount();

	// 내부 실행 함수
	UFUNCTION(BlueprintCallable)
	void FireHarpoon();
	void ToggleBoost();
	void ToggleNightVision();
	void StartReload();
	void OpenChargeWidget();

	void Initialize(const FFADItemDataRow& InItemMeta);
	EAction TagToAction(const FGameplayTag& Tag);
	void HandleLeftClick();
	void HandleRKey();

	
	

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(ReplicatedUsing = OnRep_Amount, EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
	
	
protected:
	UPROPERTY(EditAnywhere)
	float DrainPerSecond = 50.f;
	UPROPERTY()
	TObjectPtr<UUserWidget> ChargetWidget = nullptr;
	UPROPERTY(EditAnywhere, Category = "Equip|Projectile")
	TSubclassOf<AADProjectileBase> ProjectileClass = nullptr;
	
	uint8 bBoostActive : 1;
	uint8 bNightVisionOn : 1;
	TWeakObjectPtr<class ACharacter> OwningCharacter;
	float DefaultSpeed = 0.f;
	int32 MaxMagazine = 0;

	EAction LeftAction;
	EAction RKeyAction;

private:

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion

		
};
