// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameGuideInfoData.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameGuideEntryUpdatedFromDataDelegate, class UGameGuideListSlot* /*UGameGuideListSlot*/);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UGameGuideInfoData : public UObject
{
	GENERATED_BODY()
	
public:
	void Init(int32 NewSlotId, FName NewTitle);

	FOnGameGuideEntryUpdatedFromDataDelegate OnGameGuideEntryUpdatedFromDataDelegate;

protected:
	int32 SlotId;
	FName Title;

public:
	int32 GetSlotIndex() const { return SlotId; }
	FName GetTitle() const { return Title; }
};
