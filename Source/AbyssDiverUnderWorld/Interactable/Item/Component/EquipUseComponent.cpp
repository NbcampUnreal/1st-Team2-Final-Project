#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/ADProjectileBase.h"

// Sets default values for this component's properties
UEquipUseComponent::UEquipUseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	Amount = 0;
	DrainPerSecond = 50.f;
	bBoostActive = false;
}


// Called when the game starts
void UEquipUseComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UEquipUseComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	bBoostActive = false;
	Super::EndPlay(Reason);
}


// Called every frame
void UEquipUseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBoostActive && Amount > 0)
	{
		Amount = FMath::Max(0, Amount - FMath::RoundToInt(DrainPerSecond * DeltaTime));
		if (Amount == 0)
		{
			ToggleBoost(); // 베터리 고갈 -> 자동 해제
		}
	}
}

void UEquipUseComponent::S_LeftClick_Implementation()
{
	HandleLeftClick();
}

void UEquipUseComponent::S_RKey_Implementation()
{
	HandleRKey();
}

void UEquipUseComponent::OnRep_Amount()
{
	// HUD 업데이트 브로드캐스트
}

void UEquipUseComponent::Initialize(const FFADItemDataRow& InItemMeta)
{
	Amount = InItemMeta.Amount;
	//LeftAction = TagToAction(InItemMeta.LeftClickTag);
	//RKeyAction = TagToAction(InItemMeta.RKeyTag);

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwningCharacter = Char;
		DefaultSpeed = Char->GetCharacterMovement()->MaxWalkSpeed;
	}
}

EAction UEquipUseComponent::TagToAction(const FGameplayTag& Tag)
{
	//if (Tag.MatchesTagExact(TAG_Weapon_Fire))       return EAction::WeaponFire;
	//else if (Tag.MatchesTagExact(TAG_Weapon_Reload))     return EAction::WeaponReload;
	//else if (Tag.MatchesTagExact(TAG_Equip_Boost))       return EAction::ToggleBoost;
	//else if (Tag.MatchesTagExact(TAG_Equip_NVGToggle))   return EAction::ToggleNVGToggle;
	//else if (Tag.MatchesTagExact(TAG_Equip_ChargeUI))    return EAction::ApplyChargeUI;
	return EAction::None;
}

void UEquipUseComponent::HandleLeftClick()
{
	if (!GetOwner()->HasAuthority())
	{
		S_LeftClick();
		return;
	}

	switch (LeftAction)
	{
	case EAction::WeaponFire:     FireHarpoon();       break;
	case EAction::ToggleBoost:     ToggleBoost();       break;
	case EAction::ToggleNVGToggle: ToggleNightVision(); break;
	default:                      break;
	}
}

void UEquipUseComponent::HandleRKey()
{
	if (!GetOwner()->HasAuthority())
	{
		S_RKey();
		return;
	}
	switch (RKeyAction)
	{
	case EAction::WeaponReload:   StartReload();     break;
	case EAction::ApplyChargeUI:  OpenChargeWidget();break;
	default:                      break;
	}
}

void UEquipUseComponent::FireHarpoon()
{
	if (Amount <= 0 || !ProjectileClass) return;

	const FName SocketName(TEXT("FireLocationSocket"));
	FVector Loc;
	FRotator Rot;

	if (const USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh())
	{
		Loc = MeshComp->GetSocketLocation(SocketName);
		Rot = MeshComp->GetSocketRotation(SocketName);
	}
	else
	{
		Loc = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.f;
		Rot = GetOwner()->GetActorRotation();
	}

	FActorSpawnParameters Params;
	Params.Instigator = OwningCharacter.Get();
	Params.Owner = GetOwner();

	GetWorld()->SpawnActor<AADProjectileBase>(ProjectileClass, Loc, Rot, Params);

	Amount--;
	OnRep_Amount(); //서버에서도 반영
}

void UEquipUseComponent::ToggleBoost()
{
	if (!OwningCharacter.IsValid()) return;

	UCharacterMovementComponent* Move = OwningCharacter->GetCharacterMovement();
	if (!Move) return;

	if (!bBoostActive)
	{
		Move->MaxWalkSpeed *= 2.0f;
		bBoostActive = true;
	}
	else
	{
		Move->MaxWalkSpeed = DefaultSpeed;
		bBoostActive = false;
	}
}

void UEquipUseComponent::ToggleNightVision()
{
	// TODO : 포스트 프로세스 매태리얼 파라미터 토글해서 구현??
	bNightVisionOn = ~bNightVisionOn;
}

void UEquipUseComponent::StartReload()
{
	Amount = MaxMagazine;
	OnRep_Amount();
}

void UEquipUseComponent::OpenChargeWidget()
{
	//TODO 피자형 UI 생성하고 만들기

}

void UEquipUseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipUseComponent, Amount);
}

