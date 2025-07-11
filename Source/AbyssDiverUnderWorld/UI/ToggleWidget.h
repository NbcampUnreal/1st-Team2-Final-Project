#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "UI/MissionEntryWidget.h"
#include "ToggleWidget.generated.h"

class UInventoryWidget;
class UADInventoryComponent;
class URichTextBlock;
class UVerticalBox;
class USelectedMissionListWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UToggleWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeInventoriesInfo(UADInventoryComponent* InventoryComp);
    void RefreshExchangableInventoryInfo(int32 Mass, int32 Price);
    void PlaySlideAnimation(bool bIsVisible);

protected:

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    TObjectPtr <UWidgetAnimation> SlideIn;
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    TObjectPtr <UWidgetAnimation> SlideOUt;

    // 기존 인벤토리 위젯들
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> EquipmentInventory;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> ConsumableInventory;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> ExchangableInventory;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USelectedMissionListWidget> SelectedMissionList;

    // 무게, 크레딧 텍스트
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<URichTextBlock> MassText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<URichTextBlock> PriceText;

    FTimerHandle HiddenTimerHandle;

    uint8 bInventoryWidgetShowed : 1;
#pragma endregion

#pragma region Getter/Setter
public:
	UInventoryWidget* GetEquipmentInventory() const { return EquipmentInventory; }
	UInventoryWidget* GetConsumableInventory() const { return ConsumableInventory; }
	UInventoryWidget* GetExchangableInventory() const { return ExchangableInventory; }
    USelectedMissionListWidget* GetSelectedMissionList() { return SelectedMissionList; }
#pragma endregion
};
