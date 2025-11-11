// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameGuideInfoData.h"
#include "GameGuideInfoData.h"

void UGameGuideInfoData::Init(int32 NewSlotId, FName NewTitle, uint8 NewbShouldBlink)
{
	SlotId = NewSlotId;
	Title = NewTitle;
	bShouldBlink = NewbShouldBlink;
}
