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



// Sets default values for this component's properties
UEquipUseComponent::UEquipUseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
	SetIsReplicatedByDefault(true);

	Amount = 0;
	DrainPerSecond = 5.f;
	NightVisionDrainPerSecond = 2.f;
	DrainAcc = 0.f;
	bBoostActive = false;
	bOriginalExposureCached = false;
	bCanFire = true;
	bIsWeapon = true;
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

	static ConstructorHelpers::FClassFinder<UADNightVisionGoggle> NVClassFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/EquipUI/WBP_NightVisionGoggle"));
	if (NVClassFinder.Succeeded())
	{
		NightVisionClass = NVClassFinder.Class;
	}
	static ConstructorHelpers::FClassFinder<UChargeBatteryWidget> BatteryClassFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/EquipUI/WBP_ChargeBattery"));
	if (BatteryClassFinder.Succeeded())
	{
		ChargeBatteryClass = BatteryClassFinder.Class;
	}
}


// Called when the game starts
void UEquipUseComponent::BeginPlay()
{
	Super::BeginPlay();

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
	if (OwningCharacter->IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
		if (PC)
		{
			if (!NightVisionInstance && NightVisionClass)
			{
				NightVisionInstance = CreateWidget<UADNightVisionGoggle>(PC, NightVisionClass);
				if (NightVisionInstance)
				{
					NightVisionInstance->AddToViewport();
					NightVisionInstance->SetVisibility(ESlateVisibility::Hidden);
				}
			}
			if (!ChargeBatteryInstance && ChargeBatteryClass)
			{
				ChargeBatteryInstance = CreateWidget<UChargeBatteryWidget>(PC, ChargeBatteryClass);
				if (ChargeBatteryInstance)
				{
					ChargeBatteryInstance->AddToViewport();
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

	// Tick 끄기
	const bool bStillNeed = bBoostActive || bNightVisionOn || IsInterpolating();
	if (!bStillNeed)
		SetComponentTickEnabled(false);
		
}

void UEquipUseComponent::S_LeftClick_Implementation()
{
	switch (LeftAction)
	{
	case EAction::WeaponFire:      FireHarpoon();       break;
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
	case EAction::WeaponReload:   StartReload();     break;
	case EAction::ApplyChargeUI:  OpenChargeWidget();break;
	default:                      OpenChargeWidget();break;
	}
}

void UEquipUseComponent::S_IncreaseAmount_Implementation(int8 AddAmount)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		LOGIC(Log, TEXT("Initialize: No valid GameInstance"))
			return;
	}
	UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	FFADItemDataRow* ItemDataForMaxAmount = DataTableSubsystem ? DataTableSubsystem->GetItemDataByName(CurrentEquipmentName) : nullptr;
	
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
		UE_LOG(LogTemp, Error, TEXT("❌ StatusWidget is NULL"));
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
		NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 1);
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
		NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0);
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
	ToggleChargeBatteryWidget();
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

	bIsWeapon = (LeftAction == EAction::WeaponFire || RKeyAction == EAction::WeaponFire);

	

	if (bIsWeapon)
	{
		OnRep_CurrentAmmoInMag();
		OnRep_ReserveAmmo();
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
	LeftAction = EAction::None;
	RKeyAction = EAction::None;

	// 탄약/배터리 현재값 초기화
	CurrentAmmoInMag = 0;
	ReserveAmmo = 0;
	Amount = 0;

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
	if (Tag.MatchesTagExact(TAG_EquipUse_Fire))                 return EAction::WeaponFire;
	else if (Tag.MatchesTagExact(TAG_EquipUse_Reload))          return EAction::WeaponReload;
	else if (Tag.MatchesTagExact(TAG_EquipUse_DPVToggle))       return EAction::ToggleBoost;
	else if (Tag.MatchesTagExact(TAG_EquipUse_NVToggle))        return EAction::ToggleNVGToggle;
	else if (Tag.MatchesTagExact(TAG_EquipUse_ApplyChargeUI))   return EAction::ApplyChargeUI;
	return EAction::None;
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

void UEquipUseComponent::FireHarpoon()
{
	if (!bCanFire || CurrentAmmoInMag <= 0  || !ProjectileClass || !OwningCharacter.IsValid())
		return;

	FVector   CamLoc = FVector::ZeroVector;;
	FRotator  CamRot = FRotator::ZeroRotator;;
	if (AController* PC = OwningCharacter->GetController())
	{
		LOGIC(Log, TEXT("Is PlayerController"));
		PC->GetPlayerViewPoint(CamLoc, CamRot);     
	}
	else { return; }

	const FName SocketName(TEXT("FireLocationSocket"));
	FVector MuzzleLoc = CamLoc + CamRot.Vector() * 30.f;;
	if (const USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh())
	{
		if (Mesh->DoesSocketExist(SocketName))
		{
			LOGIC(Log, TEXT("Is Socket"));
			MuzzleLoc = Mesh->GetSocketLocation(SocketName);
		}
	}
	const FRotator SpawnRot = CamRot;
	const FVector LaunchDir = SpawnRot.Vector();       


	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = OwningCharacter.Get();
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//AADProjectileBase* Proj = GetWorld()->SpawnActor<AADProjectileBase>(
	//	ProjectileClass, MuzzleLoc, SpawnRot, Params);
	AADSpearGunBullet* Proj = GetWorld()->SpawnActor<AADSpearGunBullet>(
		ProjectileClass, MuzzleLoc, SpawnRot, Params);

	if (!Proj)
	{
		LOGIC(Log, TEXT("No Projectile"));
		return;
	}
	
	if (CurrentItemData->Name == "BasicSpearGun")
	{
		Proj->SetBulletType(ESpearGunType::Basic);
	}
	else if (CurrentItemData->Name == "BombSpearGun")
	{
		Proj->SetBulletType(ESpearGunType::Bomb);
	}
	else if (CurrentItemData->Name == "PoisonSpearGun")
	{
		Proj->SetBulletType(ESpearGunType::Poison);
	}

	UProjectileMovementComponent* ProjectileMovementComp = Proj->GetProjectileMovementComp();
	if (ProjectileMovementComp)
	{
		const float Speed = ProjectileMovementComp->InitialSpeed > 0
			? ProjectileMovementComp->InitialSpeed
			: 3000.f;

		ProjectileMovementComp->Velocity = LaunchDir * Speed;      
		ProjectileMovementComp->Activate(true);

		--CurrentAmmoInMag;
		OnRep_CurrentAmmoInMag();

		bCanFire = false;
		const float RefireDelay = 1.0f / RateOfFire;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_HandleRefire,
			[this]() { bCanFire = true; },
			RefireDelay, false
		);
	}
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

	bBoostActive = true;
	TargetMultiplier = BoostMultiplier;  
	SetComponentTickEnabled(true);
}

void UEquipUseComponent::BoostOff()
{
	if (!OwningCharacter.IsValid()) return;

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

void UEquipUseComponent::StartReload()
{
	if (!bIsWeapon || ReserveAmmo <= 0 || CurrentAmmoInMag == MagazineSize)
		return;
	bCanFire = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
	
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_HandleReload,
		this, &UEquipUseComponent::FinishReload,
		ReloadDuration, false
	);

}

void UEquipUseComponent::OpenChargeWidget()
{
	bChargeBatteryWidgetVisible = ~bChargeBatteryWidgetVisible;

	ToggleChargeBatteryWidget();
}

void UEquipUseComponent::FinishReload()
{
	const int32 Needed = MagazineSize - CurrentAmmoInMag;
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

void UEquipUseComponent::InitializeAmmoUI()
{
	OnRep_CurrentAmmoInMag();
	OnRep_ReserveAmmo();
}
