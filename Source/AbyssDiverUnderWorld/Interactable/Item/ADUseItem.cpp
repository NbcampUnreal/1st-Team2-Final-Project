// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/ADUseItem.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include <Net/UnrealNetwork.h>

AADUseItem::AADUseItem()
{
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Scene;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(Scene);

	SkeletalMesh->SetCollisionProfileName("BlockAllDynamic");
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);

	bReplicates = true;
}

void AADUseItem::SetItemInfo(FItemData& ItemInfo)
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* ItemDataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* ItemRow = ItemDataTableSubsystem->GetItemDataByName(ItemInfo.Name);
		if (ItemRow && ItemRow->SkeletalMesh)
		{
			ItemData = ItemInfo;
			ItemData.Quantity = 1;
			ItemData.Amount = ItemRow->Amount; //추후 사용한 양에 대한 적용 필요
			ItemData.Id = ItemRow->Id;
			M_SetSkeletalMesh(ItemRow->SkeletalMesh);
		}
	}
}

void AADUseItem::M_SetSkeletalMesh_Implementation(USkeletalMesh* NewMesh)
{
	SkeletalMesh->SetSkeletalMesh(NewMesh);
}

