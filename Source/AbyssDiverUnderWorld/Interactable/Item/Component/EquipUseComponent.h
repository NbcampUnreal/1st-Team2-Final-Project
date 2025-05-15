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
	UFUNCTION(BlueprintCallable)
	void ToggleBoost();
	UFUNCTION(BlueprintCallable)
	void ToggleNightVision();
	void ApplyManualExposure(FPostProcessSettings& PPS, float Bias);
	void RestoreOriginalExposure(FPostProcessSettings& PPS);
	void StartReload();
	void OpenChargeWidget();

	void Initialize(const FFADItemDataRow& InItemMeta);
	EAction TagToAction(const FGameplayTag& Tag);
	void HandleLeftClick();
	void HandleRKey();

	
	

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 보간 완료 확인 함수
	bool IsInterpolating() const;
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(ReplicatedUsing = OnRep_Amount, EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Boost")
	float BoostMultiplier = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = "Boost")
	float InterpSpeed = 3.f;
	
	
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
	TSubclassOf<AADProjectileBase> ProjectileClass = nullptr;
	
	
	uint8 bBoostActive : 1;
	uint8 bNightVisionOn : 1;
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

#pragma endregion

		
};
