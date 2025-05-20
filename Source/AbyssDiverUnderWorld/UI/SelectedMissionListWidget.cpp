#include "UI/SelectedMissionListWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Framework/ADPlayerState.h"
#include "Kismet/GameplayStatics.h"

void USelectedMissionListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!VerticalBox_MissionList)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ VerticalBox_MissionList 바인딩 실패"));
        return;
    }

    VerticalBox_MissionList->ClearChildren();

    // PlayerState에서 미션 가져오기
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>())
        {
            const TArray<FMissionData>& Missions = PS->GetSelectedMissions();

            for (const FMissionData& Mission : Missions)
            {
                UTextBlock* MissionText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
                MissionText->SetText(FText::FromString(Mission.Title));
                MissionText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 24)));
                MissionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

                VerticalBox_MissionList->AddChildToVerticalBox(MissionText);
            }

            UE_LOG(LogTemp, Warning, TEXT("✅ WBP_SelectedMissionList에 %d개 미션 표시됨"), Missions.Num());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ ADPlayerState 캐스팅 실패"));
        }
    }
}
