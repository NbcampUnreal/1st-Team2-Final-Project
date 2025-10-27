#include "UI/EndingWidget.h"
#include "Components/TextBlock.h" 
#include "TimerManager.h"

void UEndingWidget::StartTypingEffect()
{
	GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);

	bIsTypingLine2 = false;       
	TargetText = FullTextLine1;  
	CurrentDisplayText.Empty();
	TargetTextIndex = 0;

	if (TypingLine1) TypingLine1->SetText(FText::GetEmpty());
	if (TypingLine2) TypingLine2->SetText(FText::GetEmpty());

	GetWorld()->GetTimerManager().SetTimer(
		TypingTimerHandle,
		this,
		&UEndingWidget::TypeNext, 
		TypingSpeed,             
		true                    
	);
}

void UEndingWidget::TypeNext()
{
	if (TargetTextIndex >= TargetText.Len())
	{
		if (bIsTypingLine2)
		{
			GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
			return;
		}
		else
		{
			bIsTypingLine2 = true; 
			TargetText = FullTextLine2; 
			CurrentDisplayText.Empty();
			TargetTextIndex = 0;

			if (TargetText.IsEmpty())
			{
				GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
				return;
			}
		}
	}

	if (TargetText.IsValidIndex(TargetTextIndex))
	{
		CurrentDisplayText.AppendChar(TargetText[TargetTextIndex]);
		TargetTextIndex++;
	}
	else
	{
		TargetTextIndex = TargetText.Len();
		return;
	}

	if (bIsTypingLine2)
	{
		if (TypingLine2)
		{
			TypingLine2->SetText(FText::FromString(CurrentDisplayText));
		}
	}
	else
	{
		if (TypingLine1)
		{
			TypingLine1->SetText(FText::FromString(CurrentDisplayText));
		}
	}

	if (TargetTextIndex >= TargetText.Len() && bIsTypingLine2)
	{
		GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
	}
}

void UEndingWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
	Super::NativeDestruct();
}