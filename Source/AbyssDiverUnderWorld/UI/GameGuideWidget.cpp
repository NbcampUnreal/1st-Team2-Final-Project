// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameGuideWidget.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "UI/GameGuideInfoData.h"
#include "DataRow/GameGuideInfoRow.h"
#include "Components/ListView.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/HorizontalBox.h"
#include "UI/GameGuideListSlot.h"

void UGameGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateGuideList();
	SetGuideVisibility(false);
}

void UGameGuideWidget::UpdateGuideList()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance()))
	{
		TArray<FGameGuideInfoRow*> GuideInfoList= GI->GetSubsystem<UDataTableSubsystem>()->GetGameGuideInfoTableArray();

		for (FGameGuideInfoRow* GuideInfo : GuideInfoList)
		{ 
			UGameGuideInfoData* GuideData = NewObject<UGameGuideInfoData>();
			GuideData->Init(GuideInfo->GuideId, GuideInfo->GuideTitle);
			GuideData->OnGameGuideEntryUpdatedFromDataDelegate.AddUObject(this, &UGameGuideWidget::OnSlotEntryWidgetUpdated);
			GuideList->AddItem(GuideData); 
			UpdateGuideContentArray(0);
		}
	}
}

void UGameGuideWidget::UpdateGuideContentArray(int GuideId)
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetGameInstance()))
	{
		FGameGuideInfoRow* GuideInfo = GI->GetSubsystem<UDataTableSubsystem>()->GetGameGuideInfo(GuideId);
		Contents = GuideInfo->GuideContents;
		ContentsIdx = 0;
		UpdateGuideContent(ContentsIdx);
	}
}

void UGameGuideWidget::UpdateGuideContent(int ContentIdx)
{
	if (Contents.IsEmpty() || !Contents.IsValidIndex(ContentIdx)) return;
	GuideImage->SetBrushFromTexture(Contents[ContentIdx].ContentImage);
	GuideDescription->SetText(Contents[ContentIdx].ContentDescription);

	if (Contents.Num() == 1) SetVisibilityMovePageButton(true);
	else SetVisibilityMovePageButton(false);
}

void UGameGuideWidget::SetVisibilityMovePageButton(bool bIsVisible)
{
	if(bIsVisible) MovePageButton->SetVisibility(ESlateVisibility::Hidden);
	else MovePageButton->SetVisibility(ESlateVisibility::Visible);
}

void UGameGuideWidget::SetGuideVisibility(bool NewbIsVisible)
{ 
	bIsVisibility = NewbIsVisible;
	if (bIsVisibility)
	{ 
		if(this) this->SetVisibility(ESlateVisibility::Visible);  
	}
	else
	{
		if (this) this->SetVisibility(ESlateVisibility::Collapsed); 
	}
}

void UGameGuideWidget::PlayGameGuideAnimation(bool bIsInAnim)
{
	if (bIsAnimationPlaying) return;
	if (bIsInAnim)
	{
		PlayAnimation(ShowAnim);
	}
	else
	{
		PlayAnimation(HideAnim);
	}
}

void UGameGuideWidget::ToggleGuideVisibility()
{
	PlayGameGuideAnimation(!bIsVisibility);

}

void UGameGuideWidget::MovePage(bool bIsNext)
{
	if (bIsNext) ContentsIdx = FMath::Clamp(ContentsIdx + 1, 0, Contents.Num()-1);
	else ContentsIdx = FMath::Clamp(ContentsIdx - 1, 0, Contents.Num()-1);
	UpdateGuideContent(ContentsIdx);
}

void UGameGuideWidget::OnSlotEntryWidgetUpdated(UGameGuideListSlot* SlotEntryWidget)
{
	if (SlotEntryWidget)
	{
		SlotEntryWidget->OnGameGuideListSlotClickedDelegate.BindUObject(this, &UGameGuideWidget::UpdateGuideContentArray);
	}
}

void UGameGuideWidget::SetbIsAnimationPlaying(bool NewbIsAnimationPlaying)
{
	if (bIsAnimationPlaying == NewbIsAnimationPlaying) return;
	bIsAnimationPlaying = NewbIsAnimationPlaying;
}