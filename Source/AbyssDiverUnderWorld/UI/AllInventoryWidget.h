
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "UI/MissionEntryWidget.h"
#include "AllInventoryWidget.generated.h"

class UInventoryWidget;
class UADInventoryComponent;
class URichTextBlock;
class UVerticalBox;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAllInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeInventoriesInfo(UADInventoryComponent* InventoryComp);
    void RefreshExchangableInventoryInfo(int32 Mass, int32 Price);
    void RefreshMissionList(); 


protected:
    // 기존 인벤토리 위젯들
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> EquipmentInventory;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> ConsumableInventory;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UInventoryWidget> ExchangableInventory;

    UPROPERTY(EditDefaultsOnly, Category = "Mission")
    TSubclassOf<UMissionEntryWidget> MissionEntryWidgetClass;

    // 무게, 크레딧 텍스트
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<URichTextBlock> MassText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<URichTextBlock> PriceText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_SelectedMissions;


#pragma endregion

#pragma region Getter/Setter
public:
	UInventoryWidget* GetEquipmentInventory() const { return EquipmentInventory; }
	UInventoryWidget* GetConsumableInventory() const { return ConsumableInventory; }
	UInventoryWidget* GetExchangableInventory() const { return ExchangableInventory; }
#pragma endregion
};
