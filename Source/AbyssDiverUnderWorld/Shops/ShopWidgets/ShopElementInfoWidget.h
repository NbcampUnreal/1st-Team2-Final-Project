#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopElementInfoWidget.generated.h"

class URichTextBlock;

/**
 * 상점에서 아이템, 장비, 업글에 대한 정보를 띄우는 위젯
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopElementInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	

#pragma region Methods

public:

	void ChangeItemDescription(const FString& NewDescription);
	void ChangeInfoText(const FString& NewInfoText);
	// Static Mesh, Skeletal Mesh 둘 다 전달 가능하도록 UObject
	void ChangeItemMesh(const UObject* NewMesh);

	void SetDescriptionActive(bool bShouldActivate);
	void SetInfoTextActive(bool bShouldActivate);
	void SetMeshActive(bool bShouldActivate);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemMeshImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> InfoText;

#pragma endregion
};
