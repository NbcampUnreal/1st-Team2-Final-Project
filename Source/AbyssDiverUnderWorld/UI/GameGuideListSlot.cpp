// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameGuideListSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h" 
#include "UI/GameGuideInfoData.h"
#include "Subsystems/SoundSubsystem.h"

void UGameGuideListSlot::NativeOnListItemObjectSet(UObject* ListItemObject)
{  
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UGameGuideInfoData* GuideInfoData = Cast<UGameGuideInfoData>(ListItemObject);
	SetGuideInfo(GuideInfoData->GetSlotIndex(), GuideInfoData->GetTitle().ToString(), GuideInfoData->GetbShouldBlink());
	GuideInfoData->OnGameGuideEntryUpdatedFromDataDelegate.Broadcast(this);
}

FReply UGameGuideListSlot::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Replay = UUserWidget::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	 
	OnGameGuideListSlotClickedDelegate.ExecuteIfBound(GuideId);
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);

	return Replay;
} 

void UGameGuideListSlot::SetGuideInfo(int32 NewGuideId, const FString& NewGuideTitle, uint8 NewbIsBlinkSlot)
{
	GuideId = NewGuideId;
	GuideTitle->SetText(FText::FromString(NewGuideTitle)); 
	if (NewbIsBlinkSlot) SlotImage->SetBrushFromMaterial(BlinkTexture);
	else SlotImage->SetBrushFromTexture(DefaultTexture);
	if(bIsBlinkSlot != NewbIsBlinkSlot) bIsBlinkSlot = NewbIsBlinkSlot;
}
