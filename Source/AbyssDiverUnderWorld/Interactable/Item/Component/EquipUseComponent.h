#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DataRow/FADItemDataRow.h"
#include "Interactable/Item/ADUseItem.h"
#include "EquipUseComponent.generated.h"



class AADProjectileBase;
class UUserWidget;
class AADSpearGunBullet;
class UADNightVisionGoggle;

enum class EAction : uint8
{
	None,
	WeaponFire,
	WeaponReload,
	ToggleBoost,
	ToggleNVGToggle,
	ApplyChargeUI
};

USTRUCT(BlueprintType)
struct FEquipState
{
	GENERATED_BODY()

	// 비무기용 배터리/소모 상태
	UPROPERTY()
	int32 Amount = 0;

	// 무기용 탄창 상태
	UPROPERTY()
	int32 InMag = 0;
	UPROPERTY()
	int32 Reserve = 0;

	FEquipState() {}
	FEquipState(int32 InAmount) : Amount(InAmount), InMag(0), Reserve(0) {}
	FEquipState(int32 InInMag, int32 InReserve)
		: Amount(0), InMag(InInMag), Reserve(InReserve) {
	}
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
	UFUNCTION()
	void OnRep_CurrentAmmoInMag();
	UFUNCTION()
	void OnRep_ReserveAmmo();
	UFUNCTION()
	void OnRep_NightVisionOn();
	UFUNCTION()
	void OnRep_NightVisionUIVisible();

	// 내부 실행 함수
	UFUNCTION(BlueprintCallable)
	void FireHarpoon();
	UFUNCTION(BlueprintCallable)
	void ToggleBoost();
	UFUNCTION(BlueprintCallable)
	void ToggleNightVision();
	UFUNCTION(BlueprintCallable)
	void StartReload();
	UFUNCTION(BlueprintCallable)
	void OpenChargeWidget();
	UFUNCTION(BlueprintCallable)
	void HandleLeftClick();
	UFUNCTION(BlueprintCallable)
	void HandleRKey();
	
	void FinishReload();

	void Initialize(FItemData& ItemData);
	// 상태 초기화 함수
	void DeinitializeEquip();
	EAction TagToAction(const FGameplayTag& Tag);
	
	//void ResetEquipState();


	
	

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 보간 완료 확인 함수
	bool IsInterpolating() const;
	
	
#pragma endregion

#pragma region Variable
public:
	// ====== Fire =========
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmoInMag, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmoInMag = 5;

	UPROPERTY(ReplicatedUsing = OnRep_ReserveAmmo, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ReserveAmmo = 20;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MagazineSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float RateOfFire = 2.f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadDuration = 3.f;

	uint8 bCanFire : 1;
	uint8 bIsWeapon : 1;

	FTimerHandle TimerHandle_HandleRefire;
	FTimerHandle TimerHandle_HandleReload;
	// ======================
	UPROPERTY(ReplicatedUsing = OnRep_Amount, EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
	UPROPERTY(Replicated)
	uint8 bBoostActive : 1;
	UPROPERTY(ReplicatedUsing = OnRep_NightVisionOn)
	uint8 bNightVisionOn : 1;
	UPROPERTY(EditDefaultsOnly, Category = "Boost")
	float BoostMultiplier = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = "Boost")
	float InterpSpeed = 3.f;
	UPROPERTY(EditAnywhere, Category = "NightVision")
	TSubclassOf<UADNightVisionGoggle> NightVisionClass;
	UPROPERTY(VisibleAnywhere, Category = "NightVision")
	TObjectPtr<UADNightVisionGoggle> NightVisionInstance;

	UPROPERTY(ReplicatedUsing = OnRep_NightVisionUIVisible)
	uint8 bNVGWidgetVisible : 1;

	
	
protected:
	UPROPERTY(EditAnywhere)
	float DrainPerSecond = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "NightVision")
	float NightVisionDrainPerSecond = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "NightVision")
	TSoftObjectPtr<UMaterialInterface> NVGMaterial;
	UPROPERTY(EditAnywhere, Category = "NightVision")
	float ExposureBias = 1.5f;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> NightVisionMaterialInstance = nullptr;
	UPROPERTY()
	TObjectPtr<UUserWidget> ChargeWidget = nullptr;
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AADSpearGunBullet> ProjectileClass = nullptr;
	UPROPERTY()
	TMap<FName, FEquipState> AmountMap;
	UPROPERTY(EditAnywhere)
	FName CurrentRowName;
	
	FItemData* CurrentItemData = nullptr;

	TWeakObjectPtr<class ACharacter> OwningCharacter;
	float DefaultSpeed = 0.f;
	int32 MaxMagazine = 0;

	EAction LeftAction;
	EAction RKeyAction;

private:
	float CurrentMultiplier = 1.f;
	float TargetMultiplier = 1.f;
	float DrainAcc = 0.f;
	// NVG 설정 변수
	TObjectPtr<class UCameraComponent> CameraComp = nullptr;
	FPostProcessSettings OriginalPPSettings;
	uint8 bOriginalExposureCached : 1;

#pragma endregion

#pragma region Getter, Setteer
public:
	TMap<FName, FEquipState> GetAmountMap() const { return AmountMap; }
	uint8 IsBoost() const { return bBoostActive; }
	void SetCurrentRowName(FName InName) { CurrentRowName = InName; }
#pragma endregion		
};
