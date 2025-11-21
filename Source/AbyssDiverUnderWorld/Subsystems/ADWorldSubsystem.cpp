#include "Subsystems/ADWorldSubsystem.h"

#include "AbyssDiverUnderWorld.h"

#include "Framework/ADPlayerController.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

#include "EngineUtils.h"

const FString UADWorldSubsystem::MainMenuLevelName = TEXT("MainLevel");
const FString UADWorldSubsystem::CampLevelName = TEXT("Submarine_Lobby");
const FString UADWorldSubsystem::ShallowLevelName = TEXT("Shallow");
const FString UADWorldSubsystem::DeepLevelName = TEXT("DeepAbyss");

void UADWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LOGV(Log, TEXT("World Init"));

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 월드 초기화 시에는 VSM 끄고 시작 (맵 로드 중에 무거운 빌드를 유발하지 않도록)
	//SetVirtualShadowMapsEnabled(false);

	// 맵 로드가 끝난 뒤 안전한 시점에 켜기 — 지연을 주어서 로드 중의 병목을 피함
	const float DelaySeconds = 3.0f; // 필요에 따라 조절
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &UADWorldSubsystem::SafeEnableVSMDeferred);
	World->GetTimerManager().SetTimer(EnableTimerHandle, TimerDel, DelaySeconds, false);
}

void UADWorldSubsystem::Deinitialize()
{
	LOGV(Log, TEXT("World Deinit"));

	// 월드 종료/언로드 시 즉시 끄기 (테어다운 중 무거운 작업이 들어가지 않도록)
	//SafeDisableVSM();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(EnableTimerHandle);
		World->GetTimerManager().ClearTimer(RestoreTimerHandle);
	}

	Super::Deinitialize();
}

void UADWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FString WorldName;
	InWorld.GetName(WorldName);
	CurrentLevelName = WorldName;
	LOGV(Log, TEXT("%s Map Has BegunPlay"), *WorldName);

	if (MainMenuLevelName == WorldName)
	{
		return;
	}

	AADPlayerController* PlayerController = InWorld.GetFirstPlayerController <AADPlayerController>();
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetIgnoreMoveInput(false);
	// 카메라 페이드 아웃이 적용되어 있으면 원래대로 복구한다.
	PlayerController->ShowFadeIn();

	if (PlayerController->HasAuthority())
	{
		for (AADPlayerController* PC : TActorRange<AADPlayerController>(&InWorld))
		{
			if (AADPlayerState* ADPlayerState = PC->GetPlayerState<AADPlayerState>())
			{
				UADInventoryComponent* Inventory = ADPlayerState->GetInventory();
				if (Inventory)
				{
					Inventory->UnEquip();
					TArray<FItemData> Items = Inventory->GetInventoryList().Items;
					for (const FItemData& ItemData : Items)
					{
						if (ItemData.ItemType == EItemType::Exchangable)
						{
							Inventory->RemoveBySlotIndex(ItemData.SlotIndex, EItemType::Exchangable, false);
						}
					}
				}
			}
		}
	}
}

void UADWorldSubsystem::SetVirtualShadowMapsEnabled(bool bShouldEnable)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable"));
	if (CVar)
	{
		// 코드에서 설정한 것임을 명시
		CVar->Set(bShouldEnable, ECVF_SetByCode);
	}
}

void UADWorldSubsystem::SafeEnableVSMDeferred()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 1) 임시 완화: Nanite 디테일과 화면 퍼센티지를 떨어뜨려 VSM 빌드 시의 부하를 줄임
	IConsoleVariable* NaniteCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"));
	if (NaniteCVar)
	{
		NaniteCVar->Set(4, ECVF_SetByCode); // 낮은 값으로 임시 설정
	}
	IConsoleVariable* SP = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	if (SP)
	{
		SP->Set(80, ECVF_SetByCode); // 예시 값
	}

	// 2) VSM 켜기 (게임 스레드에서)
	//SetVirtualShadowMapsEnabled(true);

	// 3) 렌더 스레드 동기화: VSM 활성화 관련 렌더 명령이 큐에 들어가도록 하고, flush
	ENQUEUE_RENDER_COMMAND(TriggerVSMInit)(
		[](FRHICommandListImmediate& RHICmdList)
		{
			// no-option
		});

	FlushRenderingCommands();

	// 4) 충분한 시간이 지나면(예: 5초) 디테일을 원복 (또는 단계적으로 올리기)
	const float RestoreDelay = 5.0f; // 하드웨어/맵 복잡도에 따라 늘릴 것
	FTimerDelegate RestoreDel;
	RestoreDel.BindLambda([this]()
		{
			IConsoleVariable* NaniteCVar2 = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"));
			if (NaniteCVar2)
			{
				NaniteCVar2->Set(16, ECVF_SetByCode);
			}
			IConsoleVariable* SP2 = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
			if (SP2)
			{
				SP2->Set(100, ECVF_SetByCode);
			}
		});

	World->GetTimerManager().SetTimer(RestoreTimerHandle, RestoreDel, RestoreDelay, false);
}

void UADWorldSubsystem::SafeDisableVSM()
{
	// 렌더 명령이 끝나도록 보장 후 끄기
	ENQUEUE_RENDER_COMMAND(FlushBeforeDisable)(
		[](FRHICommandListImmediate& RHICmdList)
		{
			// no-option; 렌더 스레드에서 순서를 맞추기 위함
		});

	FlushRenderingCommands();

	SetVirtualShadowMapsEnabled(false);
}
