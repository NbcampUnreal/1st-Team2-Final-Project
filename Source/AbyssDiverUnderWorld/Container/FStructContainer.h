#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "DataRow/FADItemDataRow.h"
#include "AbyssDiverUnderWorld.h"
#include "FStructContainer.generated.h"

class UADInventoryComponent;

USTRUCT(BlueprintType)
struct FItemData : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 SlotIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmoInMag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReserveAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Mass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBulletType BulletType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Thumbnail;


	FItemData()
		: Name(NAME_None), Id(0), Quantity(0), SlotIndex(99), Amount(0), CurrentAmmoInMag(0), ReserveAmmo(0), Mass(0), Price(0), ItemType(EItemType::Max), BulletType(EBulletType::Max), Thumbnail(nullptr)
	{

	}
	FItemData(FName InName, uint8 InId, uint8 InQuantity, uint8 InSlotIndex, int32 InAmount, int32 CurrentAmmoInMag, int32 ReserveAmmo, int32 InMass, int32 InPrice, EItemType InType, EBulletType InBulletType, UTexture2D* InThumbnail)
		: Name(InName), Id(InId), Quantity(InQuantity), SlotIndex(InSlotIndex), Amount(InAmount), CurrentAmmoInMag(CurrentAmmoInMag), ReserveAmmo(ReserveAmmo), Mass(InMass), Price(InPrice), ItemType(InType), BulletType(InBulletType), Thumbnail(InThumbnail)
	{

	}
	FItemData(const FFADItemDataRow& ItemDataRow)
		: Name(ItemDataRow.Name), Id(ItemDataRow.Id), Quantity(1), SlotIndex(99), Amount(ItemDataRow.Amount), CurrentAmmoInMag(ItemDataRow.CurrentAmmoInMag), ReserveAmmo(ItemDataRow.ReserveAmmo), Mass(ItemDataRow.Weight), Price(ItemDataRow.Price), ItemType(ItemDataRow.ItemType), BulletType(ItemDataRow.BulletType), Thumbnail(ItemDataRow.Thumbnail)
	{

	}

	// 이 값이 바뀌면 Replication에 포함됨
	bool operator==(const FItemData& Other) const
	{
		return Name == Other.Name && Quantity == Other.Quantity && SlotIndex == Other.SlotIndex && ItemType == Other.ItemType && Thumbnail == Other.Thumbnail && Amount == Other.Amount && CurrentAmmoInMag == Other.CurrentAmmoInMag && ReserveAmmo == Other.ReserveAmmo && BulletType == Other.BulletType;
	}
};

USTRUCT()
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FItemData> Items;

	//Replication을 위한 필수 함수
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemData, FInventoryList>(Items, DeltaParms, *this);
	}

	void AddItem(const FItemData& Item)
	{
		FItemData NewItem;
		NewItem.Name = Item.Name;
		NewItem.Id = Item.Id;
		NewItem.Quantity = Item.Quantity;
		NewItem.SlotIndex = Item.SlotIndex;
		NewItem.Amount = Item.Amount;
		NewItem.CurrentAmmoInMag = Item.CurrentAmmoInMag;
		NewItem.ReserveAmmo = Item.ReserveAmmo;
		NewItem.Mass = Item.Mass;
		NewItem.Price = Item.Price;
		NewItem.ItemType = Item.ItemType;
		NewItem.BulletType = Item.BulletType;
		NewItem.Thumbnail = Item.Thumbnail;
		Items.Add(NewItem);
		MarkItemDirty(Items.Last());
	}

	bool UpdateQuantity(int8 ItemIndex, int8 Amount) //Update 성공 여부 반환
	{
		if (Items[ItemIndex].Quantity + Amount >= 0)
		{
			Items[ItemIndex].Quantity += Amount;
			MarkItemDirty(Items[ItemIndex]);
			return true;
		}
		else
			return false;
	}

	void UpdateAmount(int8 ItemIndex, int8 AddAmount)
	{
		Items[ItemIndex].Amount += AddAmount;
		MarkItemDirty(Items[ItemIndex]);
	}

	void SetAmount(int8 ItemIndex, int16 NewAmount)
	{
		Items[ItemIndex].Amount = NewAmount;
		MarkItemDirty(Items[ItemIndex]);
	}

	void UpdateSlotIndex(int8 ItemIndex, int16 NewIndex)
	{
		Items[ItemIndex].SlotIndex = NewIndex;
		MarkItemDirty(Items[ItemIndex]);
	}

	void RemoveItem(int8 Index)
	{
		if (Items.IsValidIndex(Index))
		{
			Items.RemoveAt(Index);
			MarkArrayDirty();
		}
	}

	void Resize(int8 NewSize)
	{
		Items.SetNum(NewSize);
		MarkArrayDirty();
	}
};

// FInventoryList의 구조체를 등록하기 위한 매크로
template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};