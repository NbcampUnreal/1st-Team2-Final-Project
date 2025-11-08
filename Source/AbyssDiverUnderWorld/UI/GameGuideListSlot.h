// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameGuideListSlot.generated.h"
 
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnGameGuideListSlotClickedDelegate, int32 /*SlotIndex*/);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UGameGuideListSlot : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	FOnGameGuideListSlotClickedDelegate OnGameGuideListSlotClickedDelegate;

private:
	void SetGuideInfo(int32 NewGuideId, const FString& NewGuideTitle);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> GuideTitle;

	int32 GuideId;
};
