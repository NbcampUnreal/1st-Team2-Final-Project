#include "UI/LevelSelectWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void ULevelSelectWidget::NativeOnInitialized()
{
    LevelButtons = { ShallowLevelButton,  AbyssLevelButton };
    LevelDescriptions = { ShallowDescription,  AbyssDescription };
    LevelIDs = { TEXT("Shallow"),     TEXT("DeepAbyss") };

    for (int32 i = 0; i < LevelButtons.Num(); ++i)
    {
        if (!LevelButtons[i]) { continue; }            // NULL safety
        ButtonToIndexMap.Add(LevelButtons[i], i);

        LevelButtons[i]->OnHovered.AddDynamic(this, &ULevelSelectWidget::ShowDescription);
        LevelButtons[i]->OnUnhovered.AddDynamic(this, &ULevelSelectWidget::HideDescription);
        LevelButtons[i]->OnClicked.AddDynamic(this, &ULevelSelectWidget::OnLevelClicked);
    }
    HideDescription();
    if (StartUI)
    {
        PlayAnimation(StartUI, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }
}

void ULevelSelectWidget::ShowDescription()
{
    int32 Index = INDEX_NONE;

    for (const TPair<UButton*, int32>& Pair : ButtonToIndexMap)
    {
        if (Pair.Key && Pair.Key->IsHovered())  
        {
            Index = Pair.Value;
            break;
        }
    }
    if (Index == INDEX_NONE)
        return;

    HideDescription();
    if (LevelDescriptions.IsValidIndex(Index) && LevelDescriptions[Index])
    {
        LevelDescriptions[Index]->SetVisibility(ESlateVisibility::Visible);
    }
}

void ULevelSelectWidget::HideDescription()
{
    for (UTextBlock* Desc : LevelDescriptions)
    {
        if (Desc) { Desc->SetVisibility(ESlateVisibility::Hidden); }
    }
}

void ULevelSelectWidget::OnLevelClicked()
{
    int32 Index = INDEX_NONE;

    // Click 시점에는 보통 Hover 상태가 유지되므로 재탐색
    for (const TPair<UButton*, int32>& Pair : ButtonToIndexMap)
    {
        if (Pair.Key && (Pair.Key->IsPressed() || Pair.Key->IsHovered()))
        {
            Index = Pair.Value;
            break;
        }
    }
    if (!LevelIDs.IsValidIndex(Index)) { return; }

    OnMapChosen.Broadcast(LevelIDs[Index]); 
    RemoveFromParent();                      
}
