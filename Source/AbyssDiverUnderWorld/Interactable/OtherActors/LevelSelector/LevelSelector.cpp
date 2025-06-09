#include "Interactable/OtherActors/LevelSelector/LevelSelector.h"

#include "AbyssDiverUnderWorld.h"
#include "UI/LevelSelectWidget.h"
#include "Framework/ADCampGameMode.h"

ALevelSelector::ALevelSelector()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
}

void ALevelSelector::BeginPlay()
{
	Super::BeginPlay();
}

void ALevelSelector::Interact_Implementation(AActor* InstigatorActor)
{
    if (APlayerController* PC = Cast<APlayerController>(InstigatorActor->GetInstigatorController()))
    {
        if (!PC->IsLocalController()) return;

        if (!LevelSelectWidget && LevelSelectWidgetClass)
        {
            LevelSelectWidget = CreateWidget<ULevelSelectWidget>(PC, LevelSelectWidgetClass);
            
        }
        if (LevelSelectWidget)
        {
            LevelSelectWidget->AddToViewport(); // UI 표시

            // 입력 모드 전환
            FInputModeGameAndUI Mode;
            Mode.SetWidgetToFocus(LevelSelectWidget->TakeWidget());
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = true;  

            if (LevelSelectWidget->OnMapChosen.IsBound()) return;
            // 델리게이트 바인딩
            LevelSelectWidget->OnMapChosen.AddUObject(this, &ALevelSelector::HandleMapChosen);
        }
    }
}

void ALevelSelector::HandleMapChosen(EMapName InLevelID)
{
    LevelID = InLevelID;

    AADCampGameMode* GM = Cast<AADCampGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    GM->SetSelectedLevel(LevelID);

    if (!LevelSelectWidget)
    {
        return;
    }
    APlayerController* PC = LevelSelectWidget->GetOwningPlayer();
    LevelSelectWidget->RemoveFromParent();
    LevelSelectWidget = nullptr;

    if (PC)
    {
        UE_LOG(LogTemp, Log, TEXT("Is PC!"));
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
    bSelectLevel = true;
}

UADInteractableComponent* ALevelSelector::GetInteractableComponent() const
{
	return InteractableComp;
}

bool ALevelSelector::IsHoldMode() const
{
	return false;
}

FString ALevelSelector::GetInteractionDescription() const
{
	return TEXT("Select Level!");
}

