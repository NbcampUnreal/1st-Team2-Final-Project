#include "Interactable/OtherActors/MissionSelectors/MissionSelector.h"

#include "AbyssDiverUnderWorld.h"
#include "UI/MissionSelectWidget.h"
#include "Subsystems/MissionSubsystem.h"

AMissionSelector::AMissionSelector()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
	ensure(InteractableComp);
}

void AMissionSelector::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	// 호스트만 사용
	if (HasAuthority() == false)
	{
		return;
	}

	if (ensureMsgf(MissionSelectWidgetClass, TEXT("미션 선택 위젯 클래스 등록 바람. From %s"), *GetName()) == false)
	{
		return;
	}

	MissionSelectWidget = CreateWidget<UMissionSelectWidget>(GetWorld(), MissionSelectWidgetClass);
	ensure(MissionSelectWidget);

	UMissionSubsystem* MissionSubsystem = GetGameInstance()->GetSubsystem<UMissionSubsystem>();
	if (ensure(MissionSubsystem) == false)
	{
		return;
	}

	MissionSelectWidget->OnStartButtonClickedDelegate.AddUObject(MissionSubsystem, &UMissionSubsystem::ReceiveMissionDataFromUIData);
}

void AMissionSelector::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMissionSelector::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority() == false)
	{
		LOGV(Warning, TEXT("호스트만 미션 선택 가능"));
		return;
	}

	OpenMissionSelectWidget();
}

void AMissionSelector::OpenMissionSelectWidget()
{
	MissionSelectWidget->AddToViewport();
}

UADInteractableComponent* AMissionSelector::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AMissionSelector::IsHoldMode() const
{
	return false;
}

FString AMissionSelector::GetInteractionDescription() const
{	
	// 임시
	return TEXT("Select Mission!");
}

