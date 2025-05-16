#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/ADProjectileBase.h"
#include "GameplayTagContainer.h"
#include "GameplayTags/EquipNativeTags.h"
#include "AbyssDiverUnderWorld.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/DataTableSubsystem.h"

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

	// 테스트용
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwningCharacter = Char;
		DefaultSpeed = Char->GetCharacterMovement()->MaxSwimSpeed;
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
	case EAction::ToggleBoost:     ToggleBoost();       break;
	case EAction::ToggleNVGToggle: ToggleNightVision(); break;
	default:                      break;
	}
}

void UEquipUseComponent::S_RKey_Implementation()
{
	switch (RKeyAction)
	{
	case EAction::WeaponReload:   StartReload();     break;
	case EAction::ApplyChargeUI:  OpenChargeWidget();break;
	default:                      break;
	}
}

void UEquipUseComponent::OnRep_Amount()
{
	// HUD 업데이트 브로드캐스트
}

void UEquipUseComponent::OnRep_CurrentAmmoInMag()
{
	// HUD와 UI에 탄알 수 갱신
}

void UEquipUseComponent::OnRep_ReserveAmmo()
{
	// HUD와 UI에 탄알 수 갱신
}

void UEquipUseComponent::OnRep_NightVisionOn()
{
	const float Target = bNightVisionOn ? 1.f : 0.f;
	NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), Target);
	SetComponentTickEnabled(bNightVisionOn || bBoostActive);
}

void UEquipUseComponent::Initialize(uint8 ItemId)
{
	LOG(TEXT("UEquipUseComponent::Initialize START – ItemId=%d"), ItemId);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("Initialize: No valid GameInstance"));
		return;
	}
	UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemData(ItemId) : nullptr;
	if (!InItemMeta)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipUseComponent::Initialize – Invalid ItemId %d"), ItemId);
		return;
	}
	const FName RowName = InItemMeta->Name;

	// 기존 장비의 효과 해제
	if (bBoostActive || bNightVisionOn || CurrentMultiplier != 1.f)
	{
		DeinitializeEquip();
	}

	//// 해제 전에 상태 저장
	//if (!CurrentRowName.IsNone())
	//{
	//	if (bIsWeapon)
	//	{
	//		AmountMap.FindOrAdd(CurrentRowName) = FEquipState(CurrentAmmoInMag, ReserveAmmo);
	//	}
	//	else
	//	{
	//		AmountMap.FindOrAdd(CurrentRowName) = FEquipState(Amount);
	//	}
	//}

	// 새 장착 아이템
	CurrentRowName = RowName;
	
	// Action Type 결정
	LeftAction = TagToAction(InItemMeta->LeftTag);
	RKeyAction = TagToAction(InItemMeta->RKeyTag);
	bIsWeapon = (LeftAction == EAction::WeaponFire || RKeyAction == EAction::WeaponFire);
	LOG(TEXT("Initialize: LeftAction=%d, RKeyAction=%d, bIsWeapon=%s"),
		(int32)LeftAction, (int32)RKeyAction, bIsWeapon ? TEXT("true") : TEXT("false"));

	// 저장된 총량 가져오기
	if (const FEquipState* Saved = AmountMap.Find(CurrentRowName))
	{
		if (bIsWeapon)
		{
			CurrentAmmoInMag = Saved->InMag;
			ReserveAmmo = Saved->Reserve;
		}
		else
		{
			Amount = Saved->Amount;
		}
	}
	else
	{
		// 첫 장착 시 기본값
		if (bIsWeapon)
		{
			CurrentAmmoInMag = FMath::Min(InItemMeta->Amount, MagazineSize);
			ReserveAmmo = InItemMeta->Amount - CurrentAmmoInMag;
		}
		else
		{
			Amount = InItemMeta->Amount;
		}
	}
	LOG(TEXT("Initialize: bIsWeapon=%s, Total=%d, InMag=%d, Reserve=%d"),
		bIsWeapon ? TEXT("true") : TEXT("false"),
		Amount, CurrentAmmoInMag, ReserveAmmo);

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwningCharacter = Char;
		DefaultSpeed = Char->GetCharacterMovement()->MaxSwimSpeed;
	}

	// 서버에서 HUD 동기화
	if (bIsWeapon)
	{
		OnRep_CurrentAmmoInMag();
		OnRep_ReserveAmmo();
	}
	else
	{
		OnRep_Amount();
	}
}

void UEquipUseComponent::DeinitializeEquip()
{
	if (!CurrentRowName.IsNone())
	{
		if (bIsWeapon)
		{
			AmountMap.FindOrAdd(CurrentRowName) = FEquipState(CurrentAmmoInMag, ReserveAmmo);
		}
		else
		{
			AmountMap.FindOrAdd(CurrentRowName) = FEquipState(Amount);
		}
	}


	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleRefire);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleReload);
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

	// 현재 장착 아이템 키 제거
	CurrentRowName = NAME_None;

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
		LOG(TEXT("Is PlayerController"));
		PC->GetPlayerViewPoint(CamLoc, CamRot);     
	}
	else { return; }

	const FName SocketName(TEXT("FireLocationSocket"));
	FVector MuzzleLoc = CamLoc + CamRot.Vector() * 30.f;;
	if (const USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh())
	{
		if (Mesh->DoesSocketExist(SocketName))
		{
			LOG(TEXT("Is Socket"));
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

	AADProjectileBase* Proj = GetWorld()->SpawnActor<AADProjectileBase>(
		ProjectileClass, MuzzleLoc, SpawnRot, Params);

	UProjectileMovementComponent* ProjectileMovementComp = Proj->GetProjectileMovementComp();
	if (Proj && ProjectileMovementComp)
	{
		const float Speed = ProjectileMovementComp->InitialSpeed > 0
			? ProjectileMovementComp->InitialSpeed
			: 3000.f;

		ProjectileMovementComp->Velocity = LaunchDir * Speed;      
		ProjectileMovementComp->Activate(true);

		--CurrentAmmoInMag;
		OnRep_Amount(); 

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
	
	// Tick 활성 : 비활성
	const bool bStillNeed = bBoostActive || bNightVisionOn || IsInterpolating();
	SetComponentTickEnabled(bStillNeed);
}

void UEquipUseComponent::ToggleNightVision()
{
	if (!NightVisionMaterialInstance || !CameraComp) return;
	if (Amount <= 0 || bBoostActive) return;
	
	bNightVisionOn = !bNightVisionOn;
	const float Target = bNightVisionOn ? 1.f : 0.f;
	NightVisionMaterialInstance->SetScalarParameterValue("NightBlend", Target);

	const bool bStillNeed = bBoostActive || bNightVisionOn;
	SetComponentTickEnabled(bStillNeed);
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
	//TODO 피자형 UI 생성하고 만들기

}

void UEquipUseComponent::FinishReload()
{
	const int32 Needed = MagazineSize - CurrentAmmoInMag;
	const int32 ToReload = FMath::Min(Needed, ReserveAmmo);
	
	CurrentAmmoInMag += ToReload;
	ReserveAmmo -= ToReload;
	bCanFire = true;
}

void UEquipUseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipUseComponent, Amount);
	DOREPLIFETIME(UEquipUseComponent, CurrentAmmoInMag);
	DOREPLIFETIME(UEquipUseComponent, ReserveAmmo);
	DOREPLIFETIME(UEquipUseComponent, bBoostActive);
	DOREPLIFETIME(UEquipUseComponent, bNightVisionOn);
}

bool UEquipUseComponent::IsInterpolating() const
{
	return !FMath::IsNearlyEqual(CurrentMultiplier, TargetMultiplier, 0.001f);
}

//void UEquipUseComponent::ResetEquipState()
//{
//	bBoostActive = false;
//	bNightVisionOn = false;
//
//	CurrentMultiplier = TargetMultiplier = 1.f;
//	if (OwningCharacter.IsValid())
//	{
//		OwningCharacter->GetCharacterMovement()->MaxSwimSpeed = DefaultSpeed;
//	}
//
//	if (NightVisionMaterialInstance)
//	{
//		NightVisionMaterialInstance->SetScalarParameterValue(TEXT("NightBlend"), 0.f);
//	}
//	if (CameraComp)
//	{
//		CameraComp->PostProcessSettings = OriginalPPSettings;
//	}
//
//	SetComponentTickEnabled(false);
//}
