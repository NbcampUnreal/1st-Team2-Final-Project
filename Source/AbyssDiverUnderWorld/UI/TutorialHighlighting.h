// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialHighlighting.generated.h"


USTRUCT(BlueprintType)
struct ABYSSDIVERUNDERWORLD_API FHighlightingInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAnchors Anchors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Offset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiply = 1.0f;

	FHighlightingInfo() {};
	FHighlightingInfo(const FAnchors& NewAnchors, const FVector2D& NewOffset, float NewMultiply = 1.f)
		: Anchors(NewAnchors), Offset(NewOffset), Multiply(NewMultiply) {
	};
};

class UCanvasPanel;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UTutorialHighlighting : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void HighlightStart(const FHighlightingInfo& Info);
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void HighlightEnd();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> MovingCanvas;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> ShowHighlighting;
};
