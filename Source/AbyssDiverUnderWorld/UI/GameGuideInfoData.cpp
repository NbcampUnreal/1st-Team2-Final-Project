// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameGuideInfoData.h"
#include "GameGuideInfoData.h"

void UGameGuideInfoData::Init(int32 NewSlotId, FName NewTitle)
{
	SlotId = NewSlotId;
	Title = NewTitle;
}
