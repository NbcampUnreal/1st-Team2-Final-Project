#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopItemMeshPanel.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopItemMeshPanel : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

#pragma region Methods

public:

	void Init(USkeletalMeshComponent* NewItemMeshComp);
	void ChangeItemMesh(USkeletalMesh* NewMesh);

	void SetItemMeshActive(bool bShouldActivate);

#pragma endregion

#pragma region Variables

private:

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> ItemMeshComponent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemMeshImage;

#pragma endregion

};
