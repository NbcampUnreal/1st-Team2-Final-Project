#include "UI/RadarWidgets/RadarReturnWidget.h"

#include "Components/Image.h"
#include "Components/Overlay.h"

void URadarReturnWidget::SetReturnImageColor(FColor NewColor)
{
	if (IsValid(RadarReturnImage) == false)
	{
		return;
	}

	RadarReturnImage->SetColorAndOpacity(FLinearColor(NewColor));
}
