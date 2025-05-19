#include "UI/AllInventoryWidget.h"
#include "InventoryWidget.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/RichTextBlock.h"
#include "Kismet/GameplayStatics.h"

void UAllInventoryWidget::InitializeInventoriesInfo(UADInventoryComponent* InventoryComp)
{
    const TArray<int8>& InventorySizeByType = InventoryComp->GetInventorySizeByType();
    InventoryComp->InventoryInfoUpdateDelegate.AddUObject(this, &UAllInventoryWidget::RefreshExchangableInventoryInfo);
    RefreshExchangableInventoryInfo(0, 0);

    if (EquipmentInventory && ConsumableInventory && ExchangableInventory)
    {
        EquipmentInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Equipment)], EItemType::Equipment, InventoryComp);
        ConsumableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Consumable)], EItemType::Consumable, InventoryComp);
        ExchangableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Exchangable)], EItemType::Exchangable, InventoryComp);
    }
}

void UAllInventoryWidget::RefreshExchangableInventoryInfo(int32 Mass, int32 Price)
{
    if (MassText && PriceText)
    {
        MassText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Mass)));
        PriceText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Price)));
    }
}

void UAllInventoryWidget::RefreshMissionList()
{
    if (!VerticalBox_SelectedMissions)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ VerticalBox_SelectedMissions 바인딩 실패"));
        return;
    }

    VerticalBox_SelectedMissions->ClearChildren();

    // ✅ 강제 텍스트만 추가
    UTextBlock* TestText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
    TestText->SetText(FText::FromString(TEXT("🧪 테스트 텍스트 직접 추가")));
    TestText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    TestText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 32)));
    VerticalBox_SelectedMissions->AddChildToVerticalBox(TestText);

    // ✅ Entry 추가 테스트
    if (MissionEntryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("💡 MissionEntryWidgetClass 유효함: %s"), *MissionEntryWidgetClass->GetName());
        UMissionEntryWidget* Entry = CreateWidget<UMissionEntryWidget>(GetWorld(), MissionEntryWidgetClass);

        if (Entry)
        {
            Entry->Setup(FMissionData{
                TEXT("🔥 미션 테스트 표시"), 1, true, TEXT("")
                });

            UVerticalBoxSlot* TextSlot = VerticalBox_SelectedMissions->AddChildToVerticalBox(Entry);
            if (TextSlot)
            {
                TextSlot->SetPadding(FMargin(4.0f));
                TextSlot->SetSize(ESlateSizeRule::Automatic);
            }

            UE_LOG(LogTemp, Warning, TEXT("🎯 Entry 위젯 추가됨: %s"), *Entry->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Entry 위젯 생성 실패"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ MissionEntryWidgetClass가 null"));
    }
}
