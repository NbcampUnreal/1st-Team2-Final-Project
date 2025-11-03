#include "Interactable/Item/Component/EquipUseComponent.h"

#include "AbyssDiverUnderWorld.h"

#include "Projectile/ADProjectileBase.h"
#include "Projectile/ADSpearGunBullet.h"
#include "Projectile/ADFlareGunBullet.h"
#include "Projectile/ADShotgunBullet.h"

#include "Interactable/Item/Weapon/ADMine.h"

#include "UI/ADNightVisionGoggle.h"
#include "UI/ChargeBatteryWidget.h"

#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Framework/ADPlayerState.h"
#include "Framework/ADGameInstance.h"

#include "Framework/ADPlayerController.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"

#include "Camera/CameraComponent.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"

#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTags/EquipNativeTags.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/CharacterMovementComponent.h"

const FName UEquipUseComponent::BASIC_SPEAR_GUN_NAME = TEXT("BasicSpearGun");

UEquipUseComponent::UEquipUseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
	SetIsReplicatedByDefault(true);

	Amount = 0;
	DrainPerSecond = 0.5f;
	NightVisionDrainPerSecond = 0.5f;
	DrainAcc = 0.f;
	bBoostActive = false;
	bOriginalExposureCached = false;
	bCanFire = true;
	bIsWeapon = true;
	bHasNoAnimation = false;
	NightVisionClass = nullptr;
	NightVisionInstance = nullptr;
	ChargeBatteryClass = nullptr;
	ChargeBatteryInstance = nullptr;
	bNVGWidgetVisible = false;
	bChargeBatteryWidgetVisible = false;
	bAlreadyCursorShowed = false;
	bIsReloading = false;

	// 테스트용
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwningCharacter = Char;
		DefaultSpeed = Char->GetCharacterMovement()->MaxSwimSpeed;
	}

	static ConstructorHelpers::FClassFinder<UADNightVisionGoggle> NVClassFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/ItemUI/WBP_NightVisionGoggle"));
	if (NVClassFinder.Succeeded())
	{
		NightVisionClass = NVClassFinder.Class;
	}
	static ConstructorHelpers::FClassFinder<UChargeBatteryWidget> BatteryClassFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/ItemUI/WBP_ChargeBattery"));
	if (BatteryClassFinder.Succeeded())
	{
		ChargeBatteryClass = BatteryClassFinder.Class;
	}
}

void UEquipUseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
	}

	// DPV
	CurrentMultiplier = 1.f;
	TargetMultiplier = 1.f;
	DefaultSpeed = OwningCharacter->GetCharacterMovement()->MaxSwimSpeed;


	// Night Vision
	if (UMaterialInterface* Base = NVGMaterial.LoadSynchronous())
	{
		NightVisionMaterialInstance = UMaterialInstanceDynamic::Create(Base, this);
		NightVisionMaterialInstance->SetScalarParameterValue("NightBlend", 0.f);
	}

	CameraComp = OwningCharacter->FindComponentByClass<UCameraComponent>(); // Getter를 사용하는 것이 좋아 보임
	if (!CameraComp) return;
	CameraComp->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, NightVisionMaterialInstance));
	OriginalPPSettings = CameraComp->PostProcessSettings;

	// 위젯 추가
	LOGN(TEXT("OwningCharacter : %s"), *OwningCharacter->GetName());
	if (OwningCharacter->IsLocallyControlled())
	{
		LOGN(TEXT("OwningCharacter : %s && Is Local Character"), *OwningCharacter->GetName());
		APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
		if (PC)
		{
			if (!NightVisionInstance && NightVisionClass)
			{
				NightVisionInstance = CreateWidget<UADNightVisionGoggle>(PC, NightVisionClass);
				if (NightVisionInstance)
				{
					NightVisionInstance->AddToViewport(-100);
					NightVisionInstance->SetIsFocusable(false);
					NightVisionInstance->SetVisibility(ESlateVisibility::Hidden);
				}
			}
			if (!ChargeBatteryInstance && ChargeBatteryClass)
			{
				ChargeBatteryInstance = CreateWidget<UChargeBatteryWidget>(PC, ChargeBatteryClass);
				if (ChargeBatteryInstance)
				{
					ChargeBatteryInstance->AddToViewport(100);
					ChargeBatteryInstance->SetIsFocusable(false);
					ChargeBatteryInstance->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FTimerHandle InitHandle;
		GetWorld()->GetTimerManager().SetTimer(InitHandle, [this]()
			{
				OnRep_CurrentAmmoInMag();
				OnRep_ReserveAmmo();
			}, 0.5f, false);
	}
}

void UEquipUseComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	bBoostActive = false;
	Super::EndPlay(Reason);
}


void UEquipUseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	LOGIC(Warning, TEXT("CurrentMultiplier : %f, TargetMultiplier : %f"), CurrentMultiplier, TargetMultiplier);
	// DPV 소모
	if (bBoostActive && Amount > 0)
	{
		DrainAcc += DrainPerSecond * DeltaTime;
	}

	// NVG 소모
	if (bNightVisionOn && Amount > 0)
	{
		DrainAcc += NightVisionDrainPerSecond * DeltaTime;
	}

	// DrainAcc 처리
	const int32 Decrease = FMath::FloorToInt(DrainAcc);
	if (Decrease > 0)
	{
		Amount = FMath::Max(0, Amount - Decrease);
		DrainAcc -= Decrease;
		OnRep_Amount();

		if (Amount == 0)
		{
			if (bBoostActive)
			{
				bBoostActive = false;
				TargetMultiplier = 1.f;
			}
			if (bNightVisionOn)
			{
				bNightVisionOn = false;
				if(NightVisionMaterialInstance) NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0.f);
			}
		}
	}

	// 부스트 속도 보간
	if (IsInterpolating())
	{
		
		CurrentMultiplier = FMath::FInterpTo(CurrentMultiplier,
			TargetMultiplier,
			DeltaTime,
			InterpSpeed);
		if (UCharacterMovementComponent* Move = OwningCharacter->GetCharacterMovement())
		{
			Move->MaxSwimSpeed = DefaultSpeed * CurrentMultiplier;
			LOGIC(Warning, TEXT("MaxSwimSpeed : %f"), Move->MaxSwimSpeed);
		}	
	}

	const bool bRecoilActive = RecoverRecoil(DeltaTime);

	// Tick 끄기
	const bool bStillNeed = bBoostActive || bNightVisionOn || IsInterpolating() || bRecoilActive;
	LOGIC(Warning, TEXT("bStillNeed: %s"), bStillNeed ? TEXT("True") : TEXT("False"));
	if (!bStillNeed)
	{
		SetComponentTickEnabled(bStillNeed);
	}
		
		
}

void UEquipUseComponent::S_LeftClick_Implementation()
{
	switch (LeftAction)
	{
	case EAction::HarpoonFire:     FireHarpoon();       break;
	case EAction::FlareFire:       FireFlare();	        break;
	case EAction::ShotgunFire:	   FireShotgun();       break;
	case EAction::ToggleBoost:     BoostOn();			break;
	case EAction::ToggleNVGToggle: ToggleNightVision(); break;
	case EAction::PlaceMine:	   PlaceMine();			break;
	case EAction::SwingHammer:     SwingHammer();		break;
	default:                      break;
	}
}

void UEquipUseComponent::S_LeftRelease_Implementation()
{
	if (bBoostActive)
	{
		BoostOff();
		LOGIC(Error, TEXT("BoostOff!!"));
	}
}

void UEquipUseComponent::S_RKey_Implementation()
{
	
	switch (RKeyAction)
	{
	case EAction::WeaponReload:   
		if (EquipType == EEquipmentType::HarpoonGun)
		{
			StartReload(HarpoonMagazineSize);
			
		}
		else if (EquipType == EEquipmentType::FlareGun)
		{
			StartReload(FlareMagazineSize);
		}		 
		else if (EquipType == EEquipmentType::Shotgun)
		{
			StartReload(ShotgunMagzineSize);
		}
		break;
	case EAction::ApplyChargeUI:  ShowChargeBatteryWidget();  break;
	case EAction::DetonateMine:   DetonateMine();			  break;
	default:                      break;
	}
}

void UEquipUseComponent::S_RKeyRelease_Implementation()
{
	if (bChargeBatteryWidgetVisible)
	{
		HideChargeBatteryWidget();
	}
}

void UEquipUseComponent::S_IncreaseAmount_Implementation(int8 AddAmount)
{
	if (CurrentEquipmentName != NAME_None)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>())
			{
				FFADItemDataRow* ItemDataForMaxAmount = DataTableSubsystem ? DataTableSubsystem->GetItemDataByName(CurrentEquipmentName) : nullptr;
				if (ItemDataForMaxAmount)
				{
					int16 NewAmount = Amount + AddAmount;
					int16 MaxAmount = ItemDataForMaxAmount->Amount;

					if (NewAmount < MaxAmount)
					{
						Amount = NewAmount;
					}
					else
					{
						Amount = MaxAmount;
					}
					SetEquipBatteryAmountText();
				}
			}
		}
	}
}

void UEquipUseComponent::M_PlayFireHarpoonSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::FireHarpoon, OwningCharacter->GetActorLocation());
}

void UEquipUseComponent::M_PlayFireShotgunSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::FireShotgun, OwningCharacter->GetActorLocation());
}

void UEquipUseComponent::C_ApplyRecoil_Implementation(const FRecoilConfig& Config)
{
	ApplyRecoil(Config);
}

void UEquipUseComponent::OnRep_Amount()
{
	SetEquipBatteryAmountText();
}

void UEquipUseComponent::OnRep_CurrentAmmoInMag()
{
	if (!OwningCharacter.IsValid()) return;

	AADPlayerController* PC = Cast<AADPlayerController>(OwningCharacter->GetController());
	if (!PC || !PC->IsLocalController()) return;

	UPlayerHUDComponent* HUDComp = PC->GetPlayerHUDComponent();
	if (!HUDComp) return;

	UPlayerStatusWidget* StatusWidget = HUDComp->GetPlayerStatusWidget();
	if (!StatusWidget)
	{
		LOGVN(Error, TEXT("❌ StatusWidget is NULL"));
		return;
	}

	StatusWidget->SetCurrentSpear(CurrentAmmoInMag);
}

void UEquipUseComponent::OnRep_ReserveAmmo()
{
	if (!OwningCharacter.IsValid()) return;

	AADPlayerController* PC = Cast<AADPlayerController>(OwningCharacter->GetController());
	if (!PC || !PC->IsLocalController()) return;

	UPlayerHUDComponent* HUDComp = PC->GetPlayerHUDComponent();
	if (!HUDComp) return;

	UPlayerStatusWidget* StatusWidget = HUDComp->GetPlayerStatusWidget();
	if (!StatusWidget) return;

	StatusWidget->SetTotalSpear(ReserveAmmo);
}


void UEquipUseComponent::OnRep_NightVisionOn()
{
	if (bNightVisionOn)
	{
		if (NightVisionMaterialInstance)
		{
			NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 1);
		}
		if (NightVisionInstance)
		{
			NightVisionInstance->NightVigionUse();
		}
		else
		{
			LOGIC(Log, TEXT("No UI Instance"))
		}

	}
	else
	{
		if (NightVisionMaterialInstance)
		{
			NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0);
		}
		if (NightVisionInstance)
		{
			NightVisionInstance->NightVigionUnUse();
		}
		else
		{
			LOGIC(Log, TEXT("No UI Instance"))
		}
	}

	//const float Target = bNightVisionOn ? 1.f : 0.f;
	//NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), Target);
	SetComponentTickEnabled(bNightVisionOn || bBoostActive);
}

void UEquipUseComponent::OnRep_NightVisionUIVisible()
{
	if (OwningCharacter != nullptr)
	{
		if (NightVisionInstance && OwningCharacter->IsLocallyControlled())
		{
			NightVisionInstance->SetVisibility(bNVGWidgetVisible
				? ESlateVisibility::Visible
				: ESlateVisibility::Hidden);
		}
		LOGIC(Log, TEXT("%s"), bNVGWidgetVisible ? TEXT("True") : TEXT("False"));
	}
	else
	{
		LOGIC(Log, TEXT("No Owning Character"));
	}
	
}

void UEquipUseComponent::OnRep_ChargeBatteryUIVisible()
{
	if (bChargeBatteryWidgetVisible)
	{
		ShowChargeBatteryWidget();
	}
	else
	{
		HideChargeBatteryWidget();
	}
	
}

void UEquipUseComponent::OnRep_BoostActive()
{
	if (bBoostActive)
	{
		DPVAudioID = GetSoundSubsystem()->PlayAt(ESFX::DPVOn, OwningCharacter->GetActorLocation(), 1.0f, true, 0.2f);
	}
	else
	{
		if (DPVAudioID)
		{
			GetSoundSubsystem()->StopAudio(DPVAudioID, true, 0.2f);
		}
	}
}

void UEquipUseComponent::Initialize(FItemData& ItemData)
{
	if (CurrentItemData)
	{
		DeinitializeEquip();
	}


	CurrentItemData = &ItemData;
	CurrentAmmoInMag = CurrentItemData->CurrentAmmoInMag;
	ReserveAmmo = CurrentItemData->ReserveAmmo;
	Amount = CurrentItemData->Amount;
	CurrentEquipmentName = CurrentItemData->Name;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		LOGIC(Log, TEXT("Initialize: No valid GameInstance"))
			return;
	}
	UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemData(CurrentItemData->Id) : nullptr;

	if (InItemMeta)
	{
		LeftAction = TagToAction(InItemMeta->LeftTag);
		RKeyAction = TagToAction(InItemMeta->RKeyTag);
	}

	EquipType = TagToEquipmentType(InItemMeta->LeftTag);
	bIsWeapon = (EquipType == EEquipmentType::HarpoonGun || EquipType == EEquipmentType::FlareGun || EquipType == EEquipmentType::Shotgun);
	bHasNoAnimation = (LeftAction == EAction::ToggleNVGToggle);
	

	if (bIsWeapon)
	{
		OnRep_CurrentAmmoInMag();
		OnRep_ReserveAmmo();
		//TODO: UI 띄우는 곳
		if (APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController()))
		{
			if (UPlayerHUDComponent* HUD = PC->FindComponentByClass<UPlayerHUDComponent>())
			{
				if (CurrentItemData->Name == SpearGunTypeNames[0])
				{
					HUD->C_SetSpearGunTypeImage(0);
				}
				else if (CurrentItemData->Name == SpearGunTypeNames[1])
				{
					HUD->C_SetSpearGunTypeImage(1);
				}
				else if (CurrentItemData->Name == SpearGunTypeNames[2])
				{
					HUD->C_SetSpearGunTypeImage(2);
				}
				HUD->M_SetSpearUIVisibility(true);
				HUD->M_UpdateSpearCount(CurrentAmmoInMag, ReserveAmmo);
			}
		}
	}
	else
	{
		OnRep_Amount();
	}
	
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwningCharacter = Char;
		DefaultSpeed = Char->GetCharacterMovement()->MaxSwimSpeed;
		if (LeftAction == EAction::ToggleNVGToggle)
		{
			if (NightVisionInstance)
			{
				NightVisionInstance->SetVisibility(ESlateVisibility::Visible);
				
			}
			bNVGWidgetVisible = true;
		}
	}
	LOGIC(Log, TEXT(
		"Initialize: Weapon=%s, Ammo=%d/%d, Amount=%d"),
		bIsWeapon ? TEXT("true") : TEXT("false"),
		CurrentAmmoInMag, ReserveAmmo, Amount);
}

void UEquipUseComponent::DeinitializeEquip()
{
	FName BackupName = CurrentEquipmentName;

	if (CurrentItemData)
	{
		CurrentItemData->Amount = Amount;
		CurrentItemData->CurrentAmmoInMag = CurrentAmmoInMag;
		CurrentItemData->ReserveAmmo = ReserveAmmo;
		CurrentItemData = nullptr;
		CurrentEquipmentName = NAME_None;
		LOGIC(Log, TEXT("No Current Item"));
	}


	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleReload);
	}

	if (LeftAction == EAction::ToggleNVGToggle)
	{
		if (NightVisionInstance)
		{
			NightVisionInstance->SetVisibility(ESlateVisibility::Hidden);

		}
		bNVGWidgetVisible = false;
	}

	// 입력·액션 상태 초기화
	bCanFire = true;
	bIsWeapon = false;
	bHasNoAnimation = true;
	LeftAction = EAction::None;
	RKeyAction = EAction::None;
	bIsReloading = false;

	// 탄약/배터리 현재값 초기화
	CurrentAmmoInMag = 0;
	ReserveAmmo = 0;
	Amount = 0;

	//TODO: UI 제거하는 함수

	if (APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UPlayerHUDComponent* HUD = PC->FindComponentByClass<UPlayerHUDComponent>())
		{
			HUD->M_SetSpearUIVisibility(false);
		}
	}

	// 부스트·야간투시 효과 끄기
	if (bNightVisionOn)
	{
		bNightVisionOn = false;
		OnRep_NightVisionOn();
		if (NightVisionInstance)
		{
			NightVisionInstance->NightVigionUnUse();
		}
	}
	bBoostActive = false;

	// 속도 복구
	if (OwningCharacter.IsValid())
	{
		OwningCharacter->GetCharacterMovement()->MaxSwimSpeed = DefaultSpeed;
	}

	// 머터리얼 파라미터·포스트프로세스 복원
	if (NightVisionMaterialInstance)
	{
		NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0.f);
	}
	if (CameraComp)
	{
		CameraComp->PostProcessSettings = OriginalPPSettings;
	}

	
	if (ChargeWidget)
	{
		// ChargeWidget 구현 후 TODO : ChargeWidget 닫기
	}

	// 틱 끄기
	SetComponentTickEnabled(false);
}

EAction UEquipUseComponent::TagToAction(const FGameplayTag& Tag)
{
	if (Tag.MatchesTagExact(TAG_EquipUse_Fire))                 return EAction::HarpoonFire;
	else if (Tag.MatchesTagExact(TAG_EquipUse_FireFlareGun))	return EAction::FlareFire;
	else if (Tag.MatchesTagExact(TAG_EquipUse_FireShotgun))		return EAction::ShotgunFire;
	else if (Tag.MatchesTagExact(TAG_EquipUse_Reload))          return EAction::WeaponReload;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DPVToggle))       return EAction::ToggleBoost;
	else if (Tag.MatchesTagExact(TAG_EquipUse_NVToggle))        return EAction::ToggleNVGToggle;
	else if (Tag.MatchesTagExact(TAG_EquipUse_ApplyChargeUI))   return EAction::ApplyChargeUI;
	else if (Tag.MatchesTagExact(TAG_EquipUse_PlaceMine))       return EAction::PlaceMine;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DetonateMine))    return EAction::DetonateMine;
	else if (Tag.MatchesTagExact(TAG_EquipUse_SwingHammer))     return EAction::SwingHammer;
	return EAction::None;
}

EEquipmentType UEquipUseComponent::TagToEquipmentType(const FGameplayTag& Tag)
{
	if (Tag.MatchesTagExact(TAG_EquipUse_Fire))              return EEquipmentType::HarpoonGun;
	else if (Tag.MatchesTagExact(TAG_EquipUse_FireFlareGun)) return EEquipmentType::FlareGun;
	else if (Tag.MatchesTagExact(TAG_EquipUse_FireShotgun))  return EEquipmentType::Shotgun;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DPVToggle))    return EEquipmentType::DPV;
	else if (Tag.MatchesTagExact(TAG_EquipUse_NVToggle))     return EEquipmentType::NightVision;
	else if (Tag.MatchesTagExact(TAG_EquipUse_PlaceMine))    return EEquipmentType::Mine;
	else if (Tag.MatchesTagExact(TAG_EquipUse_SwingHammer))  return EEquipmentType::ToyHammer;
	return EEquipmentType::Max;
}

void UEquipUseComponent::HandleLeftClick()
{
	if (!OwningCharacter.IsValid()) return;

	AADPlayerController* PC = Cast<AADPlayerController>(OwningCharacter->GetController());
	if (!PC || !PC->IsLocalController()) return;
	if (PC->bEnableClickEvents) return;

	S_LeftClick();
}

void UEquipUseComponent::HandleLeftRelease()
{
	S_LeftRelease();
}

void UEquipUseComponent::HandleRKey()
{
	S_RKey();
}

void UEquipUseComponent::HandleRKeyRelease()
{
	S_RKeyRelease();
}

void UEquipUseComponent::FireHarpoon()
{
	if (!CanFire()) return;

	// 0) 발사 사운드 스폰
	M_PlayFireHarpoonSound();

	// 1) 카메라 뷰
	FVector CamLoc; FRotator CamRot;
	GetCameraView(CamLoc, CamRot);
	const FVector AimDir = CamRot.Vector();

	// 2) 목표점 & 머즐 위치
	FVector TargetPoint = CalculateTargetPoint(CamLoc, AimDir);
	FVector MuzzleLoc = GetMuzzleLocation(CamLoc, AimDir);

	// 3) 발사체 스폰 및 초기화
	const FRotator SpawnRot = (TargetPoint - MuzzleLoc).Rotation();
	AADSpearGunBullet* Bullet = SpawnHarpoon(MuzzleLoc, SpawnRot);
	if (Bullet)
	{
		ConfigureProjectile(Bullet, TargetPoint, MuzzleLoc);
	}

	ApplyRecoil(HarpoonRecoil);
	C_ApplyRecoil(HarpoonRecoil);
}

void UEquipUseComponent::FireFlare()
{
	if (!CanFire()) return;

	M_PlayFireHarpoonSound();

	FVector CamLoc; FRotator CamRot;
	GetCameraView(CamLoc, CamRot);
	const FVector AimDir = CamRot.Vector();

	FVector TargetPoint = CalculateTargetPoint(CamLoc, AimDir);
	FVector MuzzleLoc = GetMuzzleLocation(CamLoc, AimDir);

	const FRotator SpawnRot = (TargetPoint - MuzzleLoc).Rotation();
	AADFlareGunBullet* FlareBullet = SpawnFlareBullet(MuzzleLoc, SpawnRot);
	if (FlareBullet)
	{
		ConfigureProjectile(FlareBullet, TargetPoint, MuzzleLoc);
	}

	ApplyRecoil(FlareRecoil);
	C_ApplyRecoil(FlareRecoil);
}

void UEquipUseComponent::FireShotgun()
{
	if (!CanFire()) return;
	
	/* 샷건 발사 사운드 스폰 */
	M_PlayFireShotgunSound();

	/* 시점 정보*/
	FVector CamLoc; FRotator CamRot;
	GetCameraView(CamLoc, CamRot);
	const FVector AimDir = CamRot.Vector();

	/* 머즐 위치 */
	FVector MuzzleLoc = GetMuzzleLocation(CamLoc, AimDir);

	/* 펠릿 반복 */
	const int32 PelletCount = ShotgunPelletCount;
	const float HalfAngleRad = FMath::DegreesToRadians(ShotgunSpreadAngle * 0.5f);

	for (int32 i = 0; i < PelletCount; ++i)
	{
		/* 무작위 분산 벡터 */
		FVector RandDir = FMath::VRandCone(AimDir, HalfAngleRad, HalfAngleRad);
		FRotator SpawnRot = RandDir.Rotation();

		/* 발사체 스폰 */
		AADProjectileBase* Pellet = SpawnShotgunBullet(MuzzleLoc, SpawnRot);
		if (Pellet)
		{
			/* 속도, 데미지, 수명 등 설정 */
			Pellet->InitializeSpeed(RandDir, PelletSpeed);       // 짧은 사거리
			//Pellet->SetBaseDamage(ShotgunBaseDamage / PelletCount);
		}
	}

	/* 탄약 차감 & 쿨다운 */
	--CurrentAmmoInMag;
	OnRep_CurrentAmmoInMag();

	bCanFire = false;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_HandleRefire,
		[this]() { bCanFire = true; },
		1.f / RateOfFire,
		false);


	ApplyRecoil(ShotgunRecoil);
	C_ApplyRecoil(ShotgunRecoil);
}

void UEquipUseComponent::ToggleBoost()
{
	if (!OwningCharacter.IsValid()) return;
	if (Amount <= 0 || bNightVisionOn) return;

	bBoostActive = !bBoostActive;
	TargetMultiplier = bBoostActive ? BoostMultiplier : 1.f; // 가속 : 감속
	
	const bool bStillNeed = bBoostActive || bNightVisionOn || IsInterpolating();
	SetComponentTickEnabled(bStillNeed);
}

void UEquipUseComponent::BoostOn()
{
	if (!OwningCharacter.IsValid()) return;

	if (Amount <= 0 || bNightVisionOn) return;
	DPVAudioID = GetSoundSubsystem()->PlayAt(ESFX::DPVOn, OwningCharacter->GetActorLocation(), 1.0f, true, 0.2f);
	bBoostActive = true;
	TargetMultiplier = BoostMultiplier;  
	SetComponentTickEnabled(true);
}

void UEquipUseComponent::BoostOff()
{
	if (!OwningCharacter.IsValid()) return;

	GetSoundSubsystem()->StopAudio(DPVAudioID, true, 0.2f);
	bBoostActive = false;
	TargetMultiplier = 1.f;  // 정상 속도로 복귀

	const bool bStillNeed = bNightVisionOn || IsInterpolating();
	SetComponentTickEnabled(bStillNeed);
}

void UEquipUseComponent::ToggleNightVision()
{
	if (!NightVisionMaterialInstance || !CameraComp) return;
	if (Amount <= 0 || bBoostActive) return;
	
	if (!bNightVisionOn)
	{
		if (NightVisionInstance)
		{
			NightVisionInstance->NightVigionUse();
		}

		bNightVisionOn = true;
	}
	else
	{
		if (NightVisionInstance)
		{
			NightVisionInstance->NightVigionUnUse();
		}
		bNightVisionOn = false;
	}
	//bNightVisionOn = !bNightVisionOn;
	const float Target = bNightVisionOn ? 1.f : 0.f;
	NightVisionMaterialInstance->SetScalarParameterValue("NightBlend", Target);

	const bool bStillNeed = bBoostActive || bNightVisionOn;
	SetComponentTickEnabled(bStillNeed);
}

void UEquipUseComponent::ToggleChargeBatteryWidget()
{
	if (OwningCharacter != nullptr)
	{
		if (OwningCharacter->IsLocallyControlled() && ChargeBatteryInstance)
		{
			LOGIC(Log, TEXT("%s"), bChargeBatteryWidgetVisible ? TEXT("True") : TEXT("False"));
			APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());

			if (!bChargeBatteryWidgetVisible)
			{
				ChargeBatteryInstance->SetVisibility(ESlateVisibility::Hidden);

				if(!bAlreadyCursorShowed)
					PC->bShowMouseCursor = false;
				PC->SetIgnoreLookInput(false);
				PC->SetInputMode(FInputModeGameOnly());
			}
			else
			{
				ChargeBatteryInstance->SetVisibility(ESlateVisibility::Visible);
				bAlreadyCursorShowed = PC->bShowMouseCursor;
				PC->bShowMouseCursor = true;
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(ChargeBatteryInstance->TakeWidget());

				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				InputMode.SetHideCursorDuringCapture(false);
				PC->SetIgnoreLookInput(true);
				PC->SetInputMode(InputMode);
			}
		}
	}
	else
	{
		LOGIC(Log, TEXT("No Owning Character"));
	}
}

void UEquipUseComponent::ShowChargeBatteryWidget()
{
	bChargeBatteryWidgetVisible = true;

	if (!OwningCharacter.IsValid() || !OwningCharacter->IsLocallyControlled() || !ChargeBatteryInstance)
		return;

	LOGIC(Log, TEXT("ShowChargeBatteryWidget: %s"), TEXT("Visible"));

	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (!PC) return;


	// UI 보이기
	ChargeBatteryInstance->PlayVisibleAnimation(true);

	// 커서 상태 저장 후 표시
	PC->bShowMouseCursor = true;

	// 입력 모드 전환 (Game+UI)
	if (!bBoostActive)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ChargeBatteryInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PC->SetIgnoreLookInput(true);
		PC->SetInputMode(InputMode);
	}
}

void UEquipUseComponent::HideChargeBatteryWidget()
{
	bChargeBatteryWidgetVisible = false;
	
	if (!OwningCharacter.IsValid() || !OwningCharacter->IsLocallyControlled() || !ChargeBatteryInstance)
		return;

	LOGIC(Log, TEXT("HideChargeBatteryWidget: %s"), TEXT("Hidden"));

	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (!PC) return;

	// UI 숨기기
	ChargeBatteryInstance->PlayVisibleAnimation(false);

	// 커서 복원
	PC->bShowMouseCursor = false;

	// 입력 모드 복귀 (Game Only)
	PC->SetIgnoreLookInput(false);
	PC->SetInputMode(FInputModeGameOnly());
}

void UEquipUseComponent::StartReload(int32 InMagazineSize)
{
	if (!bIsWeapon || ReserveAmmo <= 0 || CurrentAmmoInMag == InMagazineSize || bIsReloading)
		return;
	bCanFire = false;
	bIsReloading = true;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
	
	if (!OwningCharacter.IsValid())
		return;

	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(OwningCharacter);
	if (!Diver)
		return;

	const float MontageStopSeconds = 0.f;

	Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
	PlayReloadAnimation(EquipType, Diver);

	FTimerDelegate ReloadDel = FTimerDelegate::CreateUObject(
		this,
		&UEquipUseComponent::FinishReload,
		InMagazineSize, Diver
	);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_HandleReload,
		ReloadDel,
		ReloadDuration,
		false
	);
}

void UEquipUseComponent::OpenChargeWidget()
{
	bChargeBatteryWidgetVisible = ~bChargeBatteryWidgetVisible;

	ToggleChargeBatteryWidget();
}

void UEquipUseComponent::PlaceMine()
{
	if (Amount <= 0 || !MineClass) return;
	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(OwningCharacter.Get());
	if (!Diver) return;

	FActorSpawnParameters Params;
	Params.Owner = Diver;
	Params.Instigator = Diver;

	FVector SpawnLoc = Diver->GetActorLocation() + Diver->GetActorForwardVector() * 100.f;
	AADMine* MineActor = GetWorld()->SpawnActor<AADMine>(MineClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (MineActor)
	{
		PlacedMines.Add(MineActor);
		Amount = FMath::Max(0, Amount - 1);
		OnRep_Amount(); 
	}
}

void UEquipUseComponent::DetonateMine()
{
	for (AADMine* Mine : PlacedMines)
	{
		if (Mine)
		{
			Mine->Explode();
		}
	}
	PlacedMines.Empty();
}

void UEquipUseComponent::SwingHammer()
{
	if (!bCanFire || !OwningCharacter.IsValid())
		return;
	bCanFire = false;

	constexpr ECollisionChannel MonsterChannel = ECC_GameTraceChannel3;
	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(OwningCharacter);
	PlaySwingAnimation(Diver);

	const float CoolDown = 1.f / HammerRateOfSwing;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_HandleRefire,
		[this]()
		{
			bCanFire = true;
		},
		CoolDown,
		false
	);
}

void UEquipUseComponent::FinishReload(int32 InMagazineSize, AUnderwaterCharacter* Diver)
{
	const int32 Needed = InMagazineSize - CurrentAmmoInMag;
	const int32 ToReload = FMath::Min(Needed, ReserveAmmo);
	/*const float MontageStopSeconds = 0.f;*/

	CurrentAmmoInMag += ToReload;
	ReserveAmmo -= ToReload;
	bCanFire = true;

	InitializeAmmoUI();
	bIsReloading = false;

	//Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
	//PlayDrawAnimation(Diver);
}

void UEquipUseComponent::PlayReloadAnimation(EEquipmentType InEquipType, AUnderwaterCharacter* Diver)
{
	FAnimSyncState SyncState;
	SyncState.bEnableRightHandIK = true;
	SyncState.bEnableLeftHandIK = false;
	SyncState.bEnableFootIK = true;
	SyncState.bIsStrafing = false;
	switch (InEquipType)
	{
	case EEquipmentType::HarpoonGun : 
		Diver->M_PlayMontageOnBothMesh(
			HarpoonReloadMontage,
			1.0f,
			NAME_None,
			SyncState
		);
		break;
	case EEquipmentType::FlareGun :
		Diver->M_PlayMontageOnBothMesh(
			FlareReloadMontage,
			1.0f,
			NAME_None,
			SyncState
		);
		break;
	case EEquipmentType::Shotgun :
		Diver->M_PlayMontageOnBothMesh(
			FlareReloadMontage,
			1.0f,
			NAME_None,
			SyncState
		);
		break;
	default:
		break;
	}
}

void UEquipUseComponent::PlayDrawAnimation(AUnderwaterCharacter* Diver)
{
	FAnimSyncState SyncState;
	SyncState.bEnableRightHandIK = true;
	SyncState.bEnableLeftHandIK = false;
	SyncState.bEnableFootIK = true;
	SyncState.bIsStrafing = false;

	Diver->M_PlayMontageOnBothMesh(
		WeaponIdleMontage,
		1.0f,
		NAME_None,
		SyncState
	);
}

void UEquipUseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipUseComponent, Amount);
	DOREPLIFETIME(UEquipUseComponent, CurrentAmmoInMag);
	DOREPLIFETIME(UEquipUseComponent, ReserveAmmo);
	DOREPLIFETIME(UEquipUseComponent, bBoostActive);
	DOREPLIFETIME(UEquipUseComponent, bNightVisionOn);
	DOREPLIFETIME(UEquipUseComponent, bNVGWidgetVisible);
	DOREPLIFETIME(UEquipUseComponent, bChargeBatteryWidgetVisible);
	DOREPLIFETIME(UEquipUseComponent, CurrentEquipmentName);
}

void UEquipUseComponent::PlaySwingAnimation(AUnderwaterCharacter* Diver)
{
	if (!SwingMontage || !Diver) return;

	FAnimSyncState SyncState;
	SyncState.bEnableRightHandIK = true;
	SyncState.bEnableLeftHandIK = false;
	SyncState.bEnableFootIK = true;
	SyncState.bIsStrafing = false;

	Diver->M_StopAllMontagesOnBothMesh(0.f);
	Diver->M_PlayMontageOnBothMesh(
		SwingMontage,
		1.0f,
		NAME_None,
		SyncState
	);
}

bool UEquipUseComponent::IsInterpolating() const
{
	return !FMath::IsNearlyEqual(CurrentMultiplier, TargetMultiplier, 0.001f);
}

void UEquipUseComponent::SetEquipBatteryAmountText()
{
	if (OwningCharacter->IsLocallyControlled() && (bNightVisionOn || CurrentEquipmentName == "NightVisionGoggle") && NightVisionInstance)
	{
		NightVisionInstance->SetBatteryAmount(Amount);
		if (ChargeBatteryInstance)
		{
			ChargeBatteryInstance->SetEquipBatteryAmount(EChargeBatteryType::NightVisionGoggle, Amount);
		}
	}
	else if (OwningCharacter->IsLocallyControlled() && (bBoostActive || CurrentEquipmentName == "DPV") && ChargeBatteryInstance)
	{
		ChargeBatteryInstance->SetEquipBatteryAmount(EChargeBatteryType::DPV, Amount);
	}
}

void UEquipUseComponent::ApplyRecoil(const FRecoilConfig& Config)
{
	APlayerController* PC = Cast<APlayerController>(OwningCharacter.Get() ? OwningCharacter->GetController() : nullptr);
	if (!PC || !PC->IsLocalController()) 
		return;

	const float PitchKick = Config.PitchKick;
	const float YawKick = FMath::RandRange(-Config.YawKick, Config.YawKick);

	PC->AddPitchInput(-PitchKick);
	PC->AddYawInput(YawKick);

	PendingPitch += PitchKick;
	PendingYaw -= YawKick;
	ActiveRecoverySpeed = Config.RecoverySpeed;

	SetComponentTickEnabled(true);
}

bool UEquipUseComponent::RecoverRecoil(float DeltaTime)
{
	if (FMath::IsNearlyZero(PendingPitch, 0.01f) &&
		FMath::IsNearlyZero(PendingYaw, 0.01f))
	{
		PendingPitch = PendingYaw = 0.f;
		/*SetComponentTickEnabled(false);  */                   // 충분히 복구되면 중단
		return false;
	}

	const float Step = ActiveRecoverySpeed * DeltaTime;
	const float PitchStep = FMath::Clamp(PendingPitch, -Step, Step);
	const float YawStep = FMath::Clamp(PendingYaw, -Step, Step);

	APlayerController* PC = Cast<APlayerController>(OwningCharacter.Get() ? OwningCharacter->GetController() : nullptr);
	if (PC && PC->IsLocalController())
	{
		PC->AddPitchInput(PitchStep);
		PC->AddYawInput(YawStep);
	}

	PendingPitch -= PitchStep;
	PendingYaw -= YawStep;

	return true;
}

USoundSubsystem* UEquipUseComponent::GetSoundSubsystem()
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UADGameInstance* GameInstance = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
		{
			SoundSubsystemWeakPtr = GameInstance->GetSubsystem<USoundSubsystem>();
		}
	}

	return SoundSubsystemWeakPtr.Get();
}

void UEquipUseComponent::InitializeAmmoUI()
{
	OnRep_CurrentAmmoInMag();
	OnRep_ReserveAmmo();
}

bool UEquipUseComponent::IsSpearGun() const
{
	return bIsWeapon && CurrentEquipmentName == "BasicSpearGun";
}

bool UEquipUseComponent::CanFire() const
{
	return bCanFire && CurrentAmmoInMag > 0 && (SpearGunBulletClass || FlareGunBulletClass) && OwningCharacter.IsValid();
}

void UEquipUseComponent::GetCameraView(FVector& OutLoc, FRotator& OutRot) const
{
	OwningCharacter->GetController()->GetPlayerViewPoint(OutLoc, OutRot);
}

FVector UEquipUseComponent::CalculateTargetPoint(const FVector& CamLoc, const FVector& AimDir) const
{
	constexpr ECollisionChannel InteractionChannel = ECC_GameTraceChannel4;
	const FVector TraceEnd = CamLoc + AimDir * TraceMaxRange;
	FCollisionQueryParams Params(TEXT("HarpoonTrace"), true, GetOwner());

	// ① InteractionRay
	FHitResult HitInt;
	const bool bHitInt = GetWorld()->LineTraceSingleByChannel(
		HitInt, CamLoc, TraceEnd, InteractionChannel, Params);

	// ② Visibility
	FHitResult HitVis;
	const bool bHitVis = GetWorld()->LineTraceSingleByChannel(
		HitVis, CamLoc, TraceEnd, ECC_Visibility, Params);

	// ③ 두 결과 중 더 가까운 지점 선택
	if (bHitInt && bHitVis)
	{
		return (HitInt.Distance < HitVis.Distance) ? HitInt.ImpactPoint : HitVis.ImpactPoint;
	}
	if (bHitInt) return HitInt.ImpactPoint;
	if (bHitVis) return HitVis.ImpactPoint;

	// ④ 아무것도 맞히지 못하면 최대 사거리
	return TraceEnd;
}

FVector UEquipUseComponent::GetMuzzleLocation(const FVector& CamLoc, const FVector& AimDir) const
{
	FName SocketName;
	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(OwningCharacter);
	if (!Diver)
		return CamLoc + AimDir * 30.f;
	if (EquipType == EEquipmentType::HarpoonGun)
	{
		SocketName = TEXT("Muzzle");
	}
	else if (EquipType == EEquipmentType::FlareGun)
	{
		SocketName = TEXT("FlareMuzzle");
	}
	else if (EquipType == EEquipmentType::Shotgun)
	{
		SocketName = TEXT("ShotgunMuzzle");
	}
	USkeletalMeshComponent* Mesh = Diver->GetMesh1P();

	if (Mesh && Mesh->DoesSocketExist(SocketName))
	{
		LOGIC(Log, TEXT("Has MuzzleSocket : Name : %s"), *SocketName.ToString());
		LOGIC(Log, TEXT("Has MuzzleSocket : Location : %s"), *Mesh->GetSocketLocation(SocketName).ToString());
		return Mesh->GetSocketLocation(SocketName);
	}
	LOGIC(Log, TEXT("Not Have MuzzleSocket : Location : %s"), *(CamLoc + AimDir * 30.f).ToString());
	return CamLoc + AimDir * 30.f; // 없을 경우 기본 값
}

AADSpearGunBullet* UEquipUseComponent::SpawnHarpoon(const FVector& Loc, const FRotator& Rot)
{
	return SpawnBulletCommon<AADSpearGunBullet>(
		Loc, Rot, [](AADInGameMode* GM) { return GM->GetSpearGenericPool(); });
}

AADFlareGunBullet* UEquipUseComponent::SpawnFlareBullet(const FVector& Loc, const FRotator& Rot)
{
	return SpawnBulletCommon<AADFlareGunBullet>(
		Loc, Rot, [](AADInGameMode* GM) { return GM->GetFlareGenericPool(); });
}

AADShotgunBullet* UEquipUseComponent::SpawnShotgunBullet(const FVector& Loc, const FRotator& Rot)
{
	return SpawnBulletCommon<AADShotgunBullet>(
		Loc, Rot, [](AADInGameMode* GM) { return GM->GetShotgunGenericPool(); });
}



void UEquipUseComponent::ConfigureProjectile(AADProjectileBase* Proj, const FVector& TargetPoint, const FVector& MuzzleLoc)
{
	if (EquipType == EEquipmentType::HarpoonGun)
	{
		AADSpearGunBullet* SpearBullet = Cast<AADSpearGunBullet>(Proj);
		if (SpearBullet)
		{
			SelectSpearType(SpearBullet);
			FVector LaunchDir = (TargetPoint - MuzzleLoc).GetSafeNormal();
			Proj->InitializeSpeed(LaunchDir, 4000.0f);
		}
	}
	else if (EquipType == EEquipmentType::FlareGun)
	{
		FVector LaunchDir = (TargetPoint - MuzzleLoc).GetSafeNormal();
		Proj->InitializeSpeed(LaunchDir, 2000.0f);
		LOGIC(Log, TEXT("Flare completes Configuration!!"));
	}
	
	// 3) 탄약·쿨다운
	--CurrentAmmoInMag;
	OnRep_CurrentAmmoInMag();
	bCanFire = false;
	float Delay = 1.f / RateOfFire;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_HandleRefire,
		[this]() { bCanFire = true; },
		Delay, false);
}

void UEquipUseComponent::SelectSpearType(AADSpearGunBullet* Proj)
{
	if (CurrentItemData == nullptr)
	{
		LOGV(Error, TEXT("CurrentItemData == nullptr"));
		return;
	}

	if (CurrentItemData->Name.IsValid() == false)
	{
		LOGV(Error, TEXT("CurrentItemData->Name is not valid"));
		return;
	}

	FString NameString = CurrentItemData->Name.ToString();

	// FText 변수로 수정
	if (NameString == SpearGunTypeNames[0])
	{
		Proj->SetBulletType(ESpearGunType::Basic);
	}
	else if (NameString == SpearGunTypeNames[1])
	{
		Proj->SetBulletType(ESpearGunType::Poison);
	}
	else if (NameString == SpearGunTypeNames[2])
	{
		Proj->SetBulletType(ESpearGunType::Bomb);
	}
}

