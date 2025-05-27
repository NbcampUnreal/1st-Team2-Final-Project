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

#pragma region Methods

public:

	void Init(USkeletalMeshComponent* NewItemMeshComp);
	void ChangeItemMesh(USkeletalMesh* NewMesh, int32 ItemId);

	void SetItemMeshActive(bool bShouldActivate);

	void AddMeshRotationYaw(float Yaw);
	void SetMeshRotation(const FRotator& NewRotator);

#pragma endregion

#pragma region Variables

private:

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> ItemMeshComponent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemMeshImage;

	uint8 bIsMouseDown : 1;
	
	float CurrentMousePositionX = 0;

#pragma endregion

#pragma region Getters / Setters

public:

	bool GetMouseDown() const;
	void SetMouseDown(bool bNewMouseDown);

	float GetCurrentMousePositionY() const;
	void SetCurrentMousePositionX(float NewPositionX);

#pragma endregion


};
