#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopElementInfoWidget.generated.h"

class URichTextBlock;

/**
 * �������� ������, ���, ���ۿ� ���� ������ ���� ����
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopElementInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	

#pragma region Methods

public:

	void ChangeItemDescription(const FString& NewDescription);
	void ChangeInfoText(const FString& NewInfoText);
	// Static Mesh, Skeletal Mesh �� �� ���� �����ϵ��� UObject
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
