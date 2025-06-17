// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/OtherActors/Level/SelectMachine.h"
#include "Interactable/OtherActors/Level/InteractableButton.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/MissionSubsystem.h"
#include "DataRow/ButtonDataRow.h"
#include "DataRow/PhaseGoalRow.h"
#include "Framework/ADInGameMode.h"
#include "Framework/ADCampGameMode.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/WidgetComponent.h"
#include "UI/MissionSelectWidget.h"
#include "AbyssDiverUnderWorld.h"

ASelectMachine::ASelectMachine()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultComp = CreateDefaultSubobject<USceneComponent>(TEXT("Default Comp"));
	SetRootComponent(DefaultComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	MeshComp->SetupAttachment(RootComponent);

	SelectMissionWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Select Mission Comp"));
	SelectMissionWidgetComp->SetupAttachment(RootComponent);
	
	SelectMissionWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	SelectMissionWidgetComp->SetDrawSize(FVector2D(1930.0f, 1160.f));
	SelectMissionWidgetComp->SetPivot(FVector2D(0.5, 0.5));
	SelectMissionWidgetComp->SetTintColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));

	ConstructorHelpers::FClassFinder<UUserWidget> MisionSelectWidgetFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/UI/WBP_MissionSelect"));
	if (MisionSelectWidgetFinder.Succeeded())
	{
		SelectMissionWidgetComp->SetWidgetClass(MisionSelectWidgetFinder.Class);
	}
}

void ASelectMachine::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	LevelIDs = { EMapName::Description, EMapName::test1, EMapName::test2 };

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UMissionSubsystem* MissionSubsystem = GI->GetSubsystem<UMissionSubsystem>();
		UUserWidget* Widget = SelectMissionWidgetComp->GetWidget();
		if (Widget)
		{
			UMissionSelectWidget* MissionSelectWidget = Cast<UMissionSelectWidget>(Widget);
			MissionSelectWidget->OnStartButtonClickedDelegate.AddUObject(MissionSubsystem, &UMissionSubsystem::ReceiveMissionDataFromUIData);
			MissionSelectWidget->OnStartButtonClickedDelegate.AddUObject(this, &ASelectMachine::SwitchbSelectMission);
			//미션 초기화 버튼 연결 시 SwitchbSelectMission도 Bind 해줘야함
		}
		if (UDataTableSubsystem* DTSubsystem = GI->GetSubsystem<UDataTableSubsystem>())
		{
			for(int8 i = 0; i < static_cast<int8>(EButtonAction::MAX); ++i)
			{
				FButtonDataRow* ButtonData = DTSubsystem->GetButtonData(i);
				AInteractableButton* ButtonActor = GetWorld()->SpawnActor<AInteractableButton>(AInteractableButton::StaticClass());
				ButtonActor->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Button"));
				if (ButtonActor)
				{
					EButtonAction ButtonAction = static_cast<EButtonAction>(i);
					ButtonActor->SetButtonAction(ButtonAction);
					ButtonActor->SetActorRelativeTransform(ButtonData->ButtonTransform);
					ButtonActor->SetStaticMesh(ButtonData->ButtonMesh);
					switch (ButtonAction)
					{
					case EButtonAction::PrevLevel:
						ButtonActor->OnButtonPressed.BindUObject(this, &ASelectMachine::HandlePrevLevel);
						ButtonActor->SetButtonDescription(TEXT("이전"));
						break;
					case EButtonAction::NextLevel:
						ButtonActor->OnButtonPressed.BindUObject(this, &ASelectMachine::HandleNextLevel);
						ButtonActor->SetButtonDescription(TEXT("다음"));
						break;
					case EButtonAction::SelectLevel:
						GameStartButton = ButtonActor;
						ButtonActor->OnButtonPressed.BindUObject(this, &ASelectMachine::HandleTravelLevel);
						ButtonActor->SetButtonDescription(TEXT("미션과 맵을 선택해주세요."));
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

void ASelectMachine::UpdatelevelImage_Implementation()
{
}

void ASelectMachine::HandlePrevLevel(AActor* InteractInstigator)
{
	LOGV(Warning, TEXT("PrevLevel Button Pressed"));
	--CurrentLevelIndex;
	CurrentLevelIndex = FMath::Clamp(CurrentLevelIndex, 0, LevelIDs.Num() - 1);
	AutoSelectLevel(InteractInstigator);
}

void ASelectMachine::HandleNextLevel(AActor* InteractInstigator)
{
	LOGV(Warning, TEXT("NextLevel Button Pressed"));
	++CurrentLevelIndex;
	CurrentLevelIndex = FMath::Clamp(CurrentLevelIndex, 0, LevelIDs.Num() - 1);
	AutoSelectLevel(InteractInstigator);
}

void ASelectMachine::AutoSelectLevel(AActor* InteractInstigator)
{
	LOGV(Warning, TEXT("SelectLevel Button Pressed : %d"), CurrentLevelIndex);

	LevelID = static_cast<EMapName>(CurrentLevelIndex);

	AADCampGameMode* GM = Cast<AADCampGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	if (CurrentLevelIndex != 0)
	{
		GM->SetSelectedLevel(LevelID);
		bSelectLevel = true;
	}
	else
	{
		bSelectLevel = false;
	}

	UpdatelevelImage();
	UpdateState();
}

void ASelectMachine::HandleTravelLevel(AActor* InteractInstigator)
{
	if (!IsConditionMet())
	{
		LOGVN(Warning, TEXT("Condition Is not Met"));
		return;
	}

	if (AADInGameMode* InGameMode = Cast<AADInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		InGameMode->ReadyForTravelToCamp();
	}
	else if (AADCampGameMode* CampGameMode = Cast<AADCampGameMode>(GetWorld()->GetAuthGameMode()))
	{
		AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InteractInstigator);
		if (UnderwaterCharacter == nullptr)
		{
			LOGV(Error, TEXT("UnderwaterCharacter == nullptr"));
			return;
		}

		if (UnderwaterCharacter->IsLocallyControlled() == false)
		{
			return;
		}

		CampGameMode->TravelToInGameLevel();
	}
	else
	{
		LOGV(Error, TEXT("Failed to Travel Level"));
	}
}

void ASelectMachine::SwitchbSelectMission(const TArray<FMissionData>& MissionsFromUI)
{
	bSelectMission = !bSelectMission;
	UpdateState();
}

void ASelectMachine::UpdateState()
{
	if (bSelectMission && bSelectLevel)
	{
		GameStartButton->SetButtonDescription(TEXT("게임 시작하기"));
	}
	else if (bSelectMission && !bSelectLevel)
	{
		GameStartButton->SetButtonDescription(TEXT("미션 선택 완료 \n맵을 선택해주세요."));
	}
	else if (!bSelectMission && bSelectLevel)
	{
		GameStartButton->SetButtonDescription(TEXT("맵 선택 완료 \n미션을 선택해주세요."));
	}
	else
	{
		GameStartButton->SetButtonDescription(TEXT("미션과 맵을 선택해주세요."));
	}
}

bool ASelectMachine::IsConditionMet()
{
	if (bSelectMission && bSelectLevel)
	{
		return true;
	}
	return false;
}
