#include "UI/PauseWidget.h"
#include "Framework/ADPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UPauseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

    if (OutAnim)
    {
        FWidgetAnimationDynamicEvent FinishedEvent;
        FinishedEvent.BindDynamic(this, &UPauseWidget::OnOutAnumFinished);

        // 이 델리게이트를 FadeOutAnim의 Finished에 바인딩
        BindToAnimationFinished(OutAnim, FinishedEvent);
    }
}

void UPauseWidget::RequestClose()
{
    // 블루프린트에서 하던 "Play Animation with Finished"에 해당
    if (OutAnim)
    {
        PlayAnimation(OutAnim);
    }
    else
    {
        // 애니메이션이 없으면 바로 정리
        OnOutAnumFinished();
    }
}

void UPauseWidget::PlayInAnimation()
{
    if (InAnim)
    {
        PlayAnimation(InAnim);
    }
}

void UPauseWidget::OnOutAnumFinished()
{
    // 여기서는 애니메이션만 끝났다는 사실만 알리고,
    // 진짜 정리는 컨트롤러에게 맡긴다.
    if (AADPlayerController* PC = GetOwningPlayer<AADPlayerController>())
    {
        PC->HidePauseMenu();   // 아래에서 정리 코드 구현
    }
}