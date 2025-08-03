#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Radar2DWidget.generated.h"

class URadarReturn2DComponent;
class UImage;
class UOverlay;
class URadarReturnWidget;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API URadar2DWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma region Methods

protected:

	void AddReturn(URadarReturn2DComponent* ReturnComponent);
	void SpawnReturn(URadarReturn2DComponent* ReturnComponent);
	void DespawnReturn(const URadarReturn2DComponent* ReturnComponent);
	void DespawnReturnWidget(URadarReturnWidget* ReturnWidget);

	void UpdateReturn(URadarReturn2DComponent* ReturnComponent);
	
	bool HasReturn(const URadarReturn2DComponent* ReturnComponent);

	void SetProperReturnColor(URadarReturn2DComponent* ReturnComponent, URadarReturnWidget* ReturnWidget);
	void SetReturnWidgetTransform(URadarReturn2DComponent* ReturnComponent, URadarReturnWidget* ReturnWidget);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Radar2DWidgetSettings")
	TSubclassOf<URadarReturnWidget> ReturnWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> RadarOverlay;

	UPROPERTY()
	TMap<TWeakObjectPtr<URadarReturn2DComponent>, TObjectPtr<URadarReturnWidget>> ActivatedReturnWidgets;
	TQueue<TObjectPtr<URadarReturnWidget>> DeactivatedReturnImages;

	FVector2D OwnerLocation2D;
	FRotator OwnerRotation;

	float ActualRadarRadius;
	float RadarWidgetRadius;

private:

	

#pragma endregion



};
