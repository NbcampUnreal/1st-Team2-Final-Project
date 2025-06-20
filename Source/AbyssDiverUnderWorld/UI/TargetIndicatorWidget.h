#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TargetIndicatorWidget.generated.h"

class ATargetIndicatorManager;
class UVerticalBox;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UTargetIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma region Methods
public:

	void InitWidget(ATargetIndicatorManager* NewOwningManager);
	void ChangeTargetPosition(const FVector2D& NewPosition);
	void ChangeTargetImage(UTexture2D* NewIcon);
	void ChangeTargetDistanceText(int32 MeterDistance);

	void CalcAndSetCorrectTargetPosition();

	void SetVisible(bool bShouldVisible);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> TargetVerticalBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TargetImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DistanceText;

	UPROPERTY()
	TObjectPtr<ATargetIndicatorManager> OwningManager;

#pragma endregion
};
