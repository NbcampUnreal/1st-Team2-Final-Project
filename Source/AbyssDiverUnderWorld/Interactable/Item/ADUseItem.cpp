// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/ADUseItem.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include <Net/UnrealNetwork.h>
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Interactable/EquipableComponent/EquipableComponent.h"
#include "Components/SphereComponent.h"
#include "Character/UnderwaterCharacter.h"

AADUseItem::AADUseItem()
{
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMesh;
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetEnableGravity(false);
	SkeletalMesh->SetCollisionProfileName(TEXT("IgnoreOnlyPawnPhysics"));
	EquipableComp = CreateDefaultSubobject<UEquipableComponent>(TEXT("EquipableComponent"));

	DropMovement->SetActive(false);

	bReplicates = true;
	SetReplicateMovement(true);
}

void AADUseItem::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())          // 클라이언트 초기 1프레임 숨김
	{
		SetActorHiddenInGame(true);
	}

	FTimerHandle ApplyGravityTimerHandle;
	float UpdateTime = 0.01f;
	GetWorld()->GetTimerManager().SetTimer(ApplyGravityTimerHandle, [this]()
		{
			if (!bIsEquip && SkeletalMesh)
			{
				SkeletalMesh->AddForce(SpawnedItemGravity, TEXT("Root"), true);
			}
		}, UpdateTime, true);
}


void AADUseItem::M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh)
{
	SkeletalMesh->SetSkeletalMesh(NewMesh);
}

void AADUseItem::M_SetItemVisible_Implementation(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
}

void AADUseItem::SetItemInfo(FItemData& ItemInfo, bool bIsEquipMode, EEnvironmentState CurrentEnviromnent)
{

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* ItemDataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* ItemRow = ItemDataTableSubsystem->GetItemData(ItemInfo.Id);
		if (ItemRow && ItemRow->SkeletalMesh)
		{
			ItemData.Quantity = 1;
			
			ItemData.Id = ItemInfo.Id;
			ItemData.Amount = ItemInfo.Amount; //추후 사용한 양에 대한 적용 필요
			ItemData.CurrentAmmoInMag = ItemInfo.CurrentAmmoInMag;
			ItemData.ReserveAmmo = ItemInfo.ReserveAmmo;
			ItemData.Mass = ItemInfo.Mass;
			ItemData.Price = ItemInfo.Price;
			ItemData.ItemType = ItemInfo.ItemType;
			ItemData.Name = ItemInfo.Name;
			bool bIsEquipNightVisionGoggle = ItemInfo.Name == "NightVisionGoggle" && bIsEquipMode;
			if(!bIsEquipNightVisionGoggle) //나이트 비전 장착의 경우 메시 안 보이게
			{
				M_SetSkeletalMesh(ItemRow->SkeletalMesh);
			}
		}
	}
	bIsEquip = bIsEquipMode;
	if (bIsEquipMode)
	{
		M_EquipMode();
	}
	else
	{
		M_UnEquipMode();
	}
	if (CurrentEnviromnent != EEnvironmentState::MAX)
	{
		SpawnedItemGravity = CurrentEnviromnent == EEnvironmentState::Underwater ? FVector(0, 0, -50) : FVector(0, 0, -980);
	}
}

void AADUseItem::SetVariableValues(int32 InAmount, int32 InCurrentAmmo, int32 InReserveAmmo)
{
	ItemData.Amount = InAmount;
	ItemData.CurrentAmmoInMag = InCurrentAmmo;
	ItemData.ReserveAmmo = InReserveAmmo;
}

float AADUseItem::GetMeshMass() const
{
	if (IsValid(SkeletalMesh) == false)
	{
		return 0.0f;
	}

	return SkeletalMesh->GetMass();
}

void AADUseItem::M_UnEquipMode_Implementation()
{
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetCollisionProfileName(TEXT("IgnoreOnlyPawnPhysics"));
}

void AADUseItem::M_EquipMode_Implementation()
{
	SkeletalMesh->SetGenerateOverlapEvents(false);
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetCollisionProfileName("NoCollision");
}