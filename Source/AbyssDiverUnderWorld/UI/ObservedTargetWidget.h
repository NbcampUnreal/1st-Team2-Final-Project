#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObservedTargetWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UObservedTargetWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetTargetName(const FText& NewName);

	UFUNCTION(BlueprintCallable)
	void SetObservedTargetVisible(bool bVisible);

protected:
	/** 색/아이콘/애니메이션 같은 “연출”만 담당. **/

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnNameChanged(const FText& NewName);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnShow();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnHide();

	UPROPERTY(BlueprintReadOnly, Category = "Observed Target Widget", meta=(BindWidget))
	TObjectPtr<class UTextBlock> Text_TargetName;


private:
	FText CachedName;

};
