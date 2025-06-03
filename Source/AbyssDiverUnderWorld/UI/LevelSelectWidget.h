#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelSelectWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ULevelSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;

#pragma region Methods

public:
	UFUNCTION()
	void ShowDescription();

	UFUNCTION()
	void HideDescription();

	UFUNCTION()
	void OnLevelClicked();


#pragma endregion

#pragma region Variables
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapChosen, FName);
	FOnMapChosen OnMapChosen;

private:
	// (�ӽ�) �׽�Ʈ��
	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<UButton> ShallowLevelButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AbyssLevelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShallowDescription;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AbyssDescription;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> StartUI;
	
	FName ShallowLevelName = TEXT("Shallow");
	FName AbyssLevelName = TEXT("DeepAbyss");

	TArray<TObjectPtr<UButton>>				  LevelButtons;
	TArray<TObjectPtr<UTextBlock>>			  LevelDescriptions;
	TArray<FName>						      LevelIDs;          // �� �ε����� �����ϴ� LevelID
	TMap<TObjectPtr<UButton>, int32>          ButtonToIndexMap;  // Button* �� �ε���

#pragma endregion



#pragma region Getters / Setters

public:


#pragma endregion

};
