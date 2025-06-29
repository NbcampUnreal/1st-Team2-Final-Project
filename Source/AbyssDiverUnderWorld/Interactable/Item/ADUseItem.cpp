// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/ADUseItem.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include <Net/UnrealNetwork.h>
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbyssDiverUnderWorld.h"

AADUseItem::AADUseItem()
{
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMesh;
	//SkeletalMesh->SetMobility(EComponentMobility::Movable);
	//SkeletalMesh->SetIsReplicated(true);

	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetCollisionProfileName("BlockAllDynamic");

	bReplicates = true;
}

void AADUseItem::M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh)
{
	SkeletalMesh->SetSkeletalMesh(NewMesh);
}

void AADUseItem::SetItemInfo(FItemData& ItemInfo, bool bIsEquipMode)
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
	if (bIsEquipMode)
	{
		M_EquipMode();
	}
	else
	{
		M_UnEquipMode();
	}
}

void AADUseItem::SetVariableValues(int32 InAmount, int32 InCurrentAmmo, int32 InReserveAmmo)
{
	ItemData.Amount = InAmount;
	ItemData.CurrentAmmoInMag = InCurrentAmmo;
	ItemData.ReserveAmmo = InReserveAmmo;
}

void AADUseItem::M_UnEquipMode_Implementation()
{
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetCollisionProfileName("BlockAllDynamic");
}

void AADUseItem::M_EquipMode_Implementation()
{
	SkeletalMesh->SetGenerateOverlapEvents(false);
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetCollisionProfileName("NoCollision");
}
