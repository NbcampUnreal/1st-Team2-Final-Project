// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DragPreviewWidget.h"
#include "Components/Image.h"
#include "DragPreviewWidget.h"

void UDragPreviewWidget::SetPreviewInfo(UTexture2D* Thumbnail)
{
	if (PreviewImage)
	{
		PreviewImage->SetBrushFromTexture(Thumbnail);
	}
}
