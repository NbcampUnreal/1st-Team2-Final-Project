#include "Framework/SessionListWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "SessionEntryWidget.h"
#include "ADGameInstance.h"



void USessionListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Back)
	{
		Button_Back->OnClicked.AddDynamic(this, &USessionListWidget::OnBackClicked);
	}
}

void USessionListWidget::OnBackClicked()
{
	RemoveFromParent(); 
}
