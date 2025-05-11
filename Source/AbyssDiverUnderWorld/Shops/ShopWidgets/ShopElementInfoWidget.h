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

	void Init(UStaticMeshComponent* NewItemMeshComp);

	void ShowItemInfos(UStaticMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText);

	void ChangeItemDescription(const FString& NewDescription);
	void ChangeInfoText(const FString& NewInfoText);
	void ChangeItemMesh(UStaticMesh* NewMesh);

	void SetDescriptionActive(bool bShouldActivate);
	void SetInfoTextActive(bool bShouldActivate);
	void SetItemMeshActive(bool bShouldActivate);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ItemMeshComponent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemMeshImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> InfoText;

#pragma endregion
};
