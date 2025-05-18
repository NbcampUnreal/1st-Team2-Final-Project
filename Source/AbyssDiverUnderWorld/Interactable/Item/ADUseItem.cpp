// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/ADUseItem.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include <Net/UnrealNetwork.h>

AADUseItem::AADUseItem()
{
	PrimaryActorTick.bCanEverTick = true;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Scene;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(Scene);

	SkeletalMesh->SetCollisionProfileName("BlockAllDynamic");
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);

	bReplicates = true;
}

void AADUseItem::BeginPlay()
{
	Super::BeginPlay();
	//TODO : 기포 이펙트 추가
}

void AADUseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Location = GetActorLocation();
	float SubmersionDepth = WaterHeight - Location.Z;

	if (SubmersionDepth > 0) // 물속에 있음
	{
		// 물 저항 적용
		SkeletalMesh->SetLinearDamping(WaterLinearDamping); //2.0 ~ 5.0 사이
		SkeletalMesh->SetAngularDamping(WaterAngularDamping); //5.0 이상
	}
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
			ItemData.Amount = ItemInfo.Amount; //추후 사용한 양에 대한 적용 필요
			ItemData.Id = ItemInfo.Id;
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
		EquipMode();
	}
	else
	{
		UnEquipMode();
	}
}

void AADUseItem::UnEquipMode()
{
	SkeletalMesh->SetGenerateOverlapEvents(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetCollisionProfileName("BlockAllDynamic");
	SetActorTickEnabled(true);
}

void AADUseItem::EquipMode()
{
	SkeletalMesh->SetGenerateOverlapEvents(false);
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetCollisionProfileName("NoCollision");
	SetActorTickEnabled(false);
}
