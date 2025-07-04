﻿#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DataRow/FADItemDataRow.h"
#include "Interactable/Item/ADUseItem.h"
#include "EquipUseComponent.generated.h"

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	HarpoonGun = 0,
	FlareGun = 1,
	DPV = 2,
	NightVision = 3,
	Max = 4 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FRecoilConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) 
	float PitchKick = 2.0f;
	UPROPERTY(EditDefaultsOnly) 
	float YawKick = 0.4f;
	UPROPERTY(EditDefaultsOnly) 
	float RecoverySpeed = 10.0f;
};

class AUnderwaterCharacter;
class AADProjectileBase;
class UUserWidget;
class AADSpearGunBullet;
class AADFlareGunBullet;
class UADNightVisionGoggle;
class UChargeBatteryWidget;
class USoundSubsystem;

enum class EAction : uint8
{
	None,
	HarpoonFire,
	FlareFire,
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
	void S_LeftRelease();
	void S_LeftRelease_Implementation();
	UFUNCTION(Server, Reliable)
	void S_RKey();
	void S_RKey_Implementation();
	UFUNCTION(Server, Reliable)
	void S_RKeyRelease();
	void S_RKeyRelease_Implementation();
	UFUNCTION(Server, Reliable)
	void S_IncreaseAmount(int8 AddAmount);
	void S_IncreaseAmount_Implementation(int8 AddAmount);
	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayFireHarpoonSound();
	void M_PlayFireHarpoonSound_Implementation();
	UFUNCTION(Client, Reliable)
	void C_ApplyRecoil(const FRecoilConfig& Config);
	void C_ApplyRecoil_Implementation(const FRecoilConfig& Config);

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
	UFUNCTION()
	void OnRep_ChargeBatteryUIVisible();
	UFUNCTION()
	void OnRep_BoostActive();

	// 내부 실행 함수
	UFUNCTION(BlueprintCallable)
	void FireHarpoon();
	UFUNCTION(BlueprintCallable)
	void FireFlare();
	UFUNCTION(BlueprintCallable)
	void ToggleBoost();
	UFUNCTION(BlueprintCallable)
	void BoostOn();
	UFUNCTION(BlueprintCallable)
	void BoostOff();
	UFUNCTION(BlueprintCallable)
	void ToggleNightVision();
	UFUNCTION(BlueprintCallable)
	void ToggleChargeBatteryWidget();
	UFUNCTION(BlueprintCallable)
	void ShowChargeBatteryWidget();
	UFUNCTION(BlueprintCallable)
	void HideChargeBatteryWidget();
	UFUNCTION(BlueprintCallable)
	void StartReload(int32 InMagazineSize);
	UFUNCTION(BlueprintCallable)
	void OpenChargeWidget();
	UFUNCTION(BlueprintCallable)
	void HandleLeftClick();
	UFUNCTION(BlueprintCallable)
	void HandleLeftRelease();
	UFUNCTION(BlueprintCallable)
	void HandleRKey();
	UFUNCTION(BlueprintCallable)
	void HandleRKeyRelease();
	
	void FinishReload(int32 InMagazineSize, AUnderwaterCharacter* Diver);
	void PlayReloadAnimation(EEquipmentType InEquipType, AUnderwaterCharacter* Diver);
	void PlayDrawAnimation(AUnderwaterCharacter* Diver);
	void Initialize(FItemData& ItemData);
	// 상태 초기화 함수
	void DeinitializeEquip();
	EAction TagToAction(const FGameplayTag& Tag);
	EEquipmentType TagToEquipmentType(const FGameplayTag& Tag);

	//void ResetEquipState();

	void InitializeAmmoUI();

	bool IsSpearGun() const;

	// 헬퍼 함수
	bool CanFire() const;
	void GetCameraView(FVector& OutLoc, FRotator& OutRot) const;
	FVector CalculateTargetPoint(const FVector& CamLoc, const FVector& AimDir) const;
	FVector GetMuzzleLocation(const FVector& CamLoc, const FVector& AimDir) const;
	AADSpearGunBullet* SpawnHarpoon(const FVector& Loc, const FRotator& Rot);
	AADFlareGunBullet* SpawnFlareBullet(const FVector& Loc, const FRotator& Rot);
	void ConfigureProjectile(AADProjectileBase* Proj, const FVector& TargetPoint, const FVector& MuzzleLoc);
	void SelectSpearType(AADSpearGunBullet* Proj);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 보간 완료 확인 함수
	bool IsInterpolating() const;
	
	void SetEquipBatteryAmountText();

	void ApplyRecoil(const FRecoilConfig& Config);
	bool RecoverRecoil(float DeltaTime);
#pragma endregion

#pragma region Variable
public:
	// ====== Fire =========
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmoInMag, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmoInMag = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReserveAmmo, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ReserveAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 HarpoonMagazineSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 FlareMagazineSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float RateOfFire = 2.f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadDuration = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trace")
	float TraceMaxRange = 2000.f;

	uint8 bCanFire : 1;
	uint8 bIsWeapon : 1;
	uint8 bHasNoAnimation : 1;

	FTimerHandle TimerHandle_HandleRefire;
	FTimerHandle TimerHandle_HandleReload;
	// ======================
	UPROPERTY(ReplicatedUsing = OnRep_Amount, EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
	UPROPERTY(Replicated)
	FName CurrentEquipmentName;
	UPROPERTY(ReplicatedUsing = OnRep_BoostActive)
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
	UPROPERTY(EditAnywhere, Category = "Battery")
	TSubclassOf<UChargeBatteryWidget> ChargeBatteryClass;
	UPROPERTY(VisibleAnywhere, Category = "Battery")
	TObjectPtr<UChargeBatteryWidget> ChargeBatteryInstance;

	UPROPERTY(ReplicatedUsing = OnRep_NightVisionUIVisible)
	uint8 bNVGWidgetVisible : 1;
	UPROPERTY(ReplicatedUsing = OnRep_ChargeBatteryUIVisible)
	uint8 bChargeBatteryWidgetVisible : 1;
	uint8 bAlreadyCursorShowed : 1;

	// SpearType 저장
	TArray<FString> SpearGunTypeNames = { "BasicSpearGun", "PoisonSpearGun", "BombSpearGun" };

	
protected:
	UPROPERTY(EditAnywhere)
	float DrainPerSecond;
	UPROPERTY(EditDefaultsOnly, Category = "NightVision")
	float NightVisionDrainPerSecond;
	UPROPERTY(EditDefaultsOnly, Category = "NightVision")
	TSoftObjectPtr<UMaterialInterface> NVGMaterial;
	UPROPERTY(EditAnywhere, Category = "NightVision")
	float ExposureBias = 1.5f;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> NightVisionMaterialInstance = nullptr;
	UPROPERTY()
	TObjectPtr<UUserWidget> ChargeWidget = nullptr;
	UPROPERTY()
	TObjectPtr<UADInventoryComponent> Inventory = nullptr;
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AADSpearGunBullet> SpearGunBulletClass = nullptr;
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AADFlareGunBullet> FlareGunBulletClass = nullptr;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;


	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UAnimMontage> HarpoonReloadMontage;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UAnimMontage> FlareReloadMontage;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UAnimMontage> WeaponIdleMontage;


	FItemData* CurrentItemData = nullptr;

	TWeakObjectPtr<class ACharacter> OwningCharacter;
	float DefaultSpeed = 0.f;
	int32 MaxMagazine = 0;

	EAction LeftAction;
	EAction RKeyAction;
	EEquipmentType EquipType;

private:
	float CurrentMultiplier = 1.f;
	float TargetMultiplier = 1.f;
	float DrainAcc = 0.f;
	int32 DPVAudioID = 0;
	// NVG 설정 변수
	TObjectPtr<class UCameraComponent> CameraComp = nullptr;
	FPostProcessSettings OriginalPPSettings;
	uint8 bOriginalExposureCached : 1;
	static const FName BASIC_SPEAR_GUN_NAME;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	FRecoilConfig HarpoonRecoil;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	FRecoilConfig FlareRecoil;

	float ActiveRecoverySpeed = 0.f;
	float PendingPitch = 0.f;
	float PendingYaw = 0.f;

	uint8 bIsReloading : 1;

#pragma endregion

#pragma region Getter, Setteer
public:
	uint8 IsBoost() const { return bBoostActive; }
	EEquipmentType GetEquipType() { return EquipType; }

private:
	USoundSubsystem* GetSoundSubsystem();
	
#pragma endregion		
};
