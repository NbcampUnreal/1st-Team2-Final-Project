#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/ADProjectileBase.h"
#include "Projectile/ADSpearGunBullet.h"
#include "GameplayTagContainer.h"
#include "GameplayTags/EquipNativeTags.h"
#include "AbyssDiverUnderWorld.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"
#include "UI/ADNightVisionGoggle.h"
#include "UI/ChargeBatteryWidget.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"
#include "Framework/ADInGameMode.h"
#include "Projectile/GenericPool.h"
#include "Projectile/ADFlareGunBullet.h"

const FName UEquipUseComponent::BASIC_SPEAR_GUN_NAME = TEXT("BasicSpearGun");


// Sets default values for this component's properties
UEquipUseComponent::UEquipUseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
	SetIsReplicatedByDefault(true);

	Amount = 0;
	DrainPerSecond = 0.1f;
	NightVisionDrainPerSecond = 0.1f;
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


// Called when the game starts
void UEquipUseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
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
					NightVisionInstance->SetVisibility(ESlateVisibility::Hidden);
				}
			}
			if (!ChargeBatteryInstance && ChargeBatteryClass)
			{
				ChargeBatteryInstance = CreateWidget<UChargeBatteryWidget>(PC, ChargeBatteryClass);
				if (ChargeBatteryInstance)
				{
					ChargeBatteryInstance->AddToViewport();
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
				NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0.f);
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
		}	
	}

	const bool bRecoilActive = RecoverRecoil(DeltaTime);

	// Tick 끄기
	const bool bStillNeed = bBoostActive || bNightVisionOn || IsInterpolating() || bRecoilActive;
	if (!bStillNeed)
		SetComponentTickEnabled(false);
		
}

void UEquipUseComponent::S_LeftClick_Implementation()
{
	switch (LeftAction)
	{
	case EAction::HarpoonFire:     FireHarpoon();       break;
	case EAction::FlareFire:       FireFlare();	        break;
	case EAction::ToggleBoost:     BoostOn();			break;
	case EAction::ToggleNVGToggle: ToggleNightVision(); break;
	default:                      break;
	}
}

void UEquipUseComponent::S_LeftRelease_Implementation()
{
	if (bBoostActive)
	{
		BoostOff();
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
		break;
	case EAction::ApplyChargeUI:  ShowChargeBatteryWidget();  break;
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

	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (!PC) return;

	UPlayerHUDComponent* HUDComp = PC->FindComponentByClass<UPlayerHUDComponent>();
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

	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (!PC) return;

	UPlayerHUDComponent* HUDComp = PC->FindComponentByClass<UPlayerHUDComponent>();
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
		GetSoundSubsystem()->PlayAt(ESFX::DPVOn, OwningCharacter->GetActorLocation());
	}
	else
	{
		GetSoundSubsystem()->PlayAt(ESFX::DPVOff, OwningCharacter->GetActorLocation());
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
	bIsWeapon = (EquipType == EEquipmentType::HarpoonGun || EquipType == EEquipmentType::FlareGun);
	bHasNoAnimation = (LeftAction == EAction::ToggleNVGToggle);
	

	if (bIsWeapon)
	{
		OnRep_CurrentAmmoInMag();
		OnRep_ReserveAmmo();
		//TODO: UI 띄우는 곳
		if (CurrentEquipmentName == BASIC_SPEAR_GUN_NAME)
		{
			if (APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController()))
			{
				if (UPlayerHUDComponent* HUD = PC->FindComponentByClass<UPlayerHUDComponent>())
				{
					HUD->SetSpearUIVisibility(true);
					HUD->UpdateSpearCount(CurrentAmmoInMag, ReserveAmmo);
				}
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

	// 탄약/배터리 현재값 초기화
	CurrentAmmoInMag = 0;
	ReserveAmmo = 0;
	Amount = 0;

	//TODO: UI 제거하는 함수
	if (BackupName == BASIC_SPEAR_GUN_NAME)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController()))
		{
			if (UPlayerHUDComponent* HUD = PC->FindComponentByClass<UPlayerHUDComponent>())
			{
				HUD->SetSpearUIVisibility(false);
			}
		}
	}

	// 부스트·야간투시 효과 끄기
	bBoostActive = false;
	bNightVisionOn = false;

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
	else if (Tag.MatchesTagExact(TAG_EquipUse_Reload))          return EAction::WeaponReload;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DPVToggle))       return EAction::ToggleBoost;
	else if (Tag.MatchesTagExact(TAG_EquipUse_NVToggle))        return EAction::ToggleNVGToggle;
	else if (Tag.MatchesTagExact(TAG_EquipUse_ApplyChargeUI))   return EAction::ApplyChargeUI;
	return EAction::None;
}

EEquipmentType UEquipUseComponent::TagToEquipmentType(const FGameplayTag& Tag)
{
	if (Tag.MatchesTagExact(TAG_EquipUse_Fire))           return EEquipmentType::HarpoonGun;
	else if (Tag.MatchesTagExact(TAG_EquipUse_FireFlareGun)) return EEquipmentType::FlareGun;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DPVToggle))    return EEquipmentType::DPV;
	else if (Tag.MatchesTagExact(TAG_EquipUse_NVToggle))     return EEquipmentType::NightVision;
	return EEquipmentType::Max;
}

void UEquipUseComponent::HandleLeftClick()
{
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
	if (!bIsWeapon || ReserveAmmo <= 0 || CurrentAmmoInMag == InMagazineSize)
		return;
	bCanFire = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
	
	FTimerDelegate ReloadDel = FTimerDelegate::CreateUObject(
		this,
		&UEquipUseComponent::FinishReload,
		InMagazineSize
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

void UEquipUseComponent::FinishReload(int32 InMagazineSize)
{
	const int32 Needed = InMagazineSize - CurrentAmmoInMag;
	const int32 ToReload = FMath::Min(Needed, ReserveAmmo);
	
	CurrentAmmoInMag += ToReload;
	ReserveAmmo -= ToReload;
	bCanFire = true;

	InitializeAmmoUI();
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
		SetComponentTickEnabled(false);                     // 충분히 복구되면 중단
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
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
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
	FVector TraceEnd = CamLoc + AimDir * TraceMaxRange;
	FHitResult Impact;
	FCollisionQueryParams Params(TEXT("HarpoonTrace"), true, GetOwner());
	GetWorld()->LineTraceSingleByChannel(Impact, CamLoc, TraceEnd, ECC_Visibility, Params);
	return Impact.bBlockingHit ? Impact.ImpactPoint : TraceEnd;
}

FVector UEquipUseComponent::GetMuzzleLocation(const FVector& CamLoc, const FVector& AimDir) const
{
	FName SocketName;
	if (EquipType == EEquipmentType::HarpoonGun)
	{
		SocketName = TEXT("Muzzle");
	}
	else if (EquipType == EEquipmentType::FlareGun)
	{
		SocketName = TEXT("FlareMuzzle");
	}
	
	if (auto* Mesh = OwningCharacter->GetMesh();
		Mesh && Mesh->DoesSocketExist(SocketName))
	{
		return Mesh->GetSocketLocation(SocketName);
	}
	return CamLoc + AimDir * 30.f; // 없을 경우 기본 값
}

AADSpearGunBullet* UEquipUseComponent::SpawnHarpoon(const FVector& Loc, const FRotator& Rot)
{
	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (GetWorld())
	{
		AADInGameMode* GM = Cast<AADInGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			AADSpearGunBullet* SpawnedBullet = nullptr;
			AGenericPool* GenericPool = GM->GetSpearGenericPool();
			if (GenericPool)
			{
				SpawnedBullet = GenericPool->GetObject<AADSpearGunBullet>();
				if (SpawnedBullet)
				{
					APawn* PawnOwner = Cast<APawn>(PC->GetPawn());
					if (PawnOwner)
					{
						SpawnedBullet->SetInstigator(PawnOwner);
					}
					SpawnedBullet->SetOwner(PC);
					SpawnedBullet->InitializeTransform(Loc, Rot);
				}
				LOGIC(Log, TEXT("Projectile Id : %d"), SpawnedBullet->GetProjectileId());
				return SpawnedBullet;
			}
		}
	}
	return nullptr;
}

AADFlareGunBullet* UEquipUseComponent::SpawnFlareBullet(const FVector& Loc, const FRotator& Rot)
{
	APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
	if (GetWorld())
	{
		AADInGameMode* GM = Cast<AADInGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			AADFlareGunBullet* FlareBullet = nullptr;
			AGenericPool* GenericPool = GM->GetFlareGenericPool();
			if (GenericPool)
			{
				FlareBullet = GenericPool->GetObject<AADFlareGunBullet>();
				if (FlareBullet)
				{
					APawn* PawnOwner = Cast<APawn>(PC->GetPawn());
					if (PawnOwner)
					{
						FlareBullet->SetInstigator(PawnOwner);
					}
					FlareBullet->SetOwner(PC);
					FlareBullet->InitializeTransform(Loc, Rot);
				}
				return FlareBullet;
			}
		}
	}
	return nullptr;
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
	// FText 변수로 수정
	if (CurrentItemData->Name == SpearGunTypeNames[0])
	{
		Proj->SetBulletType(ESpearGunType::Basic);
	}
	else if (CurrentItemData->Name == SpearGunTypeNames[1])
	{
		Proj->SetBulletType(ESpearGunType::Poison);
	}
	else if (CurrentItemData->Name == SpearGunTypeNames[2])
	{
		Proj->SetBulletType(ESpearGunType::Bomb);
	}

}

