// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSpectatorPawn.h"

#include "EnhancedInputComponent.h"
#include "UnderwaterCharacter.h"
#include "Framework/ADPlayerController.h"

DEFINE_LOG_CATEGORY(LogAbyssDiverSpectate);

AADSpectatorPawn::AADSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AADSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay 시점에서는 Controller가 아직 Possess되지 않은 상태일 수 있으므로 관련 처리는 PossessedBy에서 처리
}

void AADSpectatorPawn::Destroyed()
{
	// 관전 중에 관전이 종료되었을 경우 현재 타겟 캐릭터의 관전 종료 이벤트를 호출한다.
	if (PrevTargetCharacter.IsValid())
	{
		PrevTargetCharacter->OnEndSpectated();
	}

	Super::Destroyed();
}

void AADSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ViewNextPlayerAction)
		{
			EnhancedInput->BindAction(
				ViewNextPlayerAction,
				ETriggerEvent::Started,
				this,
				&AADSpectatorPawn::ViewNextPlayer
			);
		}

		if (ViewPrevPlayerAction)
		{
			EnhancedInput->BindAction(
				ViewPrevPlayerAction,
				ETriggerEvent::Started,
				this,
				&AADSpectatorPawn::ViewPrevPlayer
			);
		}
	}
}

void AADSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AADPlayerController* PlayerController = Cast<AADPlayerController>(NewController))
	{
		PlayerController->OnTargetViewChanged.AddUniqueDynamic(this, &AADSpectatorPawn::OnTargetViewChanged);
		PlayerController->PlayerCameraManager->SetManualCameraFade(0.0f, FColor::Black, false);
	}
	else
	{
		UE_LOG(LogAbyssDiverSpectate, Error, TEXT("PlayerController is not valid for %s"), *GetName());
	}
}

void AADSpectatorPawn::ViewNextPlayer()
{
	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->ServerViewNextPlayer();
	}
}

void AADSpectatorPawn::ViewPrevPlayer()
{
	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->ServerViewPrevPlayer();
	}
}

void AADSpectatorPawn::OnTargetViewChanged(AActor* NewViewTarget)
{
	// Target View가 변경될 때 Player Controller(Self)를 거쳐서 전이된다. 항상 옳바른 타겟일 것이라 생각하지 않고 전이 중일 수 있는 것에 주의.
	// 전이 중인 것과 타겟이 없는 것이 구분이 되지 않으므로 처리에 유의.
	UE_LOG(LogAbyssDiverSpectate, Display, TEXT("AADSpectatorPawn::OnTargetViewChanged: NewViewTarget: %s | NetMode : %s"),
		*GetNameSafe(NewViewTarget),
		GetNetMode() == ENetMode::NM_Client ? TEXT("Client") : TEXT("Server")
	);

	if (PrevTargetCharacter.IsValid())
	{
		PrevTargetCharacter->OnEndSpectated();
	}
	
	if (NewViewTarget && NewViewTarget->IsA(AUnderwaterCharacter::StaticClass())
		&& GetController()->GetPawn() == nullptr) // 게임 입장 시의 Spectator 상태이고 Character Pawn과 동시에 존재한다. 이 상황은 관전이 아니므로 무시한다.
	{
		if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(NewViewTarget))
		{
			UnderwaterCharacter->OnCharacterStateChangedDelegate.AddUniqueDynamic(this, &AADSpectatorPawn::OnCharacterStateChanged);
			UnderwaterCharacter->OnSpectated();
			PrevTargetCharacter = UnderwaterCharacter;
		}
	}
}

void AADSpectatorPawn::OnCharacterStateChanged(ECharacterState OldCharacterState, ECharacterState NewCharacterState)
{
	if (NewCharacterState == ECharacterState::Death)
	{
		ViewNextPlayer();
	}
}
