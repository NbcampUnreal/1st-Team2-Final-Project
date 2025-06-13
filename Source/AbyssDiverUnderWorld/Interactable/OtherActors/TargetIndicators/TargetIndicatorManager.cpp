#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"

#include "AbyssDiverUnderWorld.h"
#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"
#include "UI/TargetIndicatorWidget.h"

#include "EngineUtils.h"
#include "Components/BillboardComponent.h"

ATargetIndicatorManager::ATargetIndicatorManager()
{
	BillboardSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Editor BillboardSprite"));
	SetRootComponent(BillboardSprite);

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void ATargetIndicatorManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LOGV(Error, TEXT("World == nullptr"));
		return;
	}

	if (ensureMsgf(TargetIndicatorWidgetClass, TEXT("TargetIndicatorWidgetClass를 설정해주세요")) == false)
	{
		return;
	}

	TargetIndicatorWidgetInstance = CreateWidget<UTargetIndicatorWidget>(World, TargetIndicatorWidgetClass);
	if (TargetIndicatorWidgetInstance == nullptr)
	{
		LOGV(Error, TEXT("TargetIndicatorWidgetInstance == nullptr"));
		return;
	}

	TargetIndicatorWidgetInstance->InitWidget(this);
	TargetIndicatorWidgetInstance->AddToViewport();

	int32 ManagerCount = 0;
	for (ATargetIndicatorManager* Manager : TActorRange<ATargetIndicatorManager>(World))
	{
		ManagerCount++;
		if (ensureMsgf(ManagerCount == 1, TEXT("TargetIndicatorManager는 레벨당 하나만 존재해야 합니다.")) == false)
		{
			return;
		}
	}
	
	for (AIndicatingTarget* Target : TActorRange<AIndicatingTarget>(World))
	{
		TargetArrayByReverseOrder.Emplace(Target);

		Target->OnIndicatingTargetBeginOverlapDelegate.AddUObject(this, &ATargetIndicatorManager::OnIndicatingTargetOverlapped);
	}

	Algo::Sort(TargetArrayByReverseOrder, [](const AIndicatingTarget* A, const AIndicatingTarget* B)
		{
			return A->GetTargetOrder() > B->GetTargetOrder();
		});

	AIndicatingTarget* Target = nullptr;
	if (TryGetCurrentTarget(Target) == false)
	{
		return;
	}
	
	TargetIndicatorWidgetInstance->ChangeTargetImage(Target->GetTargetIcon());
}

void ATargetIndicatorManager::OnIndicatingTargetOverlapped(int32 TargetOrder)
{
	AIndicatingTarget* Target = nullptr;
	if (TryGetCurrentTarget(Target) == false)
	{
		return;
	}

	if (Target->GetTargetOrder() != TargetOrder)
	{
		return;
	}

	TargetArrayByReverseOrder.Pop(EAllowShrinking::Yes);
	Target->Destroy();

	AIndicatingTarget* NextTarget = nullptr;
	if (TryGetCurrentTarget(NextTarget) == false)
	{
		return;
	}

	TargetIndicatorWidgetInstance->ChangeTargetImage(NextTarget->GetTargetIcon());
}

bool ATargetIndicatorManager::TryGetTargetLocation(FVector& OutLocation)
{
	AIndicatingTarget* Target = nullptr;
	if (TryGetCurrentTarget(Target) == false)
	{
		return false;
	}

	if (IsValid(Target) && Target->IsPendingKillPending() == false)
	{
		OutLocation = Target->GetActorLocation();
		return true;
	}

	return false;
}

bool ATargetIndicatorManager::TryGetCurrentTarget(AIndicatingTarget*& OutTarget)
{
	AIndicatingTarget* Target = nullptr;

	const int32 CandidateCount = TargetArrayByReverseOrder.Num();
	for (int32 i = 0; i < CandidateCount; ++i)
	{
		Target = TargetArrayByReverseOrder.Last();
		if (IsValid(Target) && Target->IsPendingKillPending() == false)
		{
			break;
		}

		TargetArrayByReverseOrder.Pop(EAllowShrinking::Yes);
	}

	OutTarget = Target;
	
	return (OutTarget && IsValid(OutTarget) && OutTarget->IsPendingKillPending() == false);
}

