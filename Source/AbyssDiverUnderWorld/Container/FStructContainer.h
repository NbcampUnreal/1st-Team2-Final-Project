#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "DataRow/FADItemDataRow.h"
#include "FStructContainer.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) 
    FName Name;

    UPROPERTY(EditAnywhere) 
	uint8 Id;

    UPROPERTY(EditAnywhere) 
	uint8 Quantity;

    UPROPERTY(EditAnywhere) 
	int32 Amount;

    UPROPERTY(EditAnywhere)
    float Mass;

	UPROPERTY(EditAnywhere)
	EItemType ItemType;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> Thumbnail;

	FItemData()
		: Name(NAME_None), Id(0), Quantity(0), Amount(0), ItemType(EItemType::Max), Thumbnail(nullptr)
	{
	}
	FItemData(FName InName, uint8 InId, uint8 InQuantity, int32 InAmount, EItemType InType, UTexture2D* InThumbnail)
		: Name(InName), Id(InId), Quantity(InQuantity), Amount(InAmount), ItemType(InType), Thumbnail(InThumbnail)
	{
	}

	// 이 값이 바뀌면 Replication에 포함됨
	bool operator==(const FItemData& Other) const
	{
		return Name == Other.Name && Quantity == Other.Quantity && ItemType == Other.ItemType && Thumbnail == Other.Thumbnail && Amount == Other.Amount;
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
		NewItem.Quantity = Item.Quantity;
		NewItem.ItemType = Item.ItemType;
		NewItem.Thumbnail = Item.Thumbnail;
		NewItem.Amount = Item.Amount;
		Items.Add(NewItem);
		MarkArrayDirty();
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