// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterCharacter.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputComponent.h"
#include "LocomotionMode.h"
#include "PlayerComponent/OxygenComponent.h"
#include "PlayerComponent/StaminaComponent.h"
#include "StatComponent.h"
#include "UpgradeComponent.h"
#include "AbyssDiverUnderWorld/Interactable/Item/Component/ADInteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/SpotLightComponent.h"
#include "Footstep/FootstepComponent.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Interactable/OtherActors/Radars/Radar.h"
#include "Inventory/ADInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shops/ShopInteractionComponent.h"
#include "Subsystems/DataTableSubsystem.h"
#include "UI/HoldInteractionWidget.h"
#include "Laser/ADLaserCutter.h"
#include "PlayerComponent/LanternComponent.h"
#include "PlayerComponent/ShieldComponent.h"

DEFINE_LOG_CATEGORY(LogAbyssDiverCharacter);

AUnderwaterCharacter::AUnderwaterCharacter()
{
	FirstPersonCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonCameraArm"));
	FirstPersonCameraArm->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraArm->bUsePawnControlRotation = true;
	FirstPersonCameraArm->TargetArmLength = 0.f; // Camera가 Mesh에 붙어있도록 한다.
	FirstPersonCameraArm->bEnableCameraRotationLag = true;
	FirstPersonCameraArm->CameraRotationLagSpeed = 10.f;
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonCameraArm);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	Mesh1PSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Mesh1PSpringArm"));
	Mesh1PSpringArm->SetupAttachment(FirstPersonCameraComponent);
	Mesh1PSpringArm->TargetArmLength = 0.0f;
	Mesh1PSpringArm->bEnableCameraRotationLag = true;
	Mesh1PSpringArm->CameraRotationLagSpeed = 10.0f;
	Mesh1PSpringArm->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(Mesh1PSpringArm);
	Mesh1P->CastShadow = false;
	Mesh1P->bCastDynamicShadow = false;
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->CastShadow = true;
	GetMesh()->bCastHiddenShadow = true;

	StatComponent->Initialize(1000, 1000, 400.0f, 10);
	
	LastLandedTime = -1.0f;
	LandedJumpBlockTime = 0.1f;
	ExpectedGravityZ = -980.0f;

	bCanUseEquipment = true;
	
	LeftFlipperSocketName = TEXT("foot_l_flipper_socket");
	RightFlipperSocketName = TEXT("foot_r_flipper_socket");
	
	LookSensitivity = 1.0f;
	NormalLookSensitivity = 1.0f;
	
	GroggyDuration = 60.0f;
	GroggyReductionRate = 0.1f;
	MinGroggyDuration = 10.0f;
	GroggyCount = 0;
	GroggyLookSensitivity = 0.25f;
	RescueRequireTime = 6.0f;
	RecoveryHealthPercentage = 1.0f;
	
	GatherMultiplier = 1.0f;
	
	bIsCaptured = false;
	CaptureFadeTime = 0.5f;

	bIsHideInSeaweed = false;
	
	BloodEmitPower = 1.0f;

	OverloadWeight = 40.0f;
	OverloadSpeedFactor = 0.4f;
	
	StatComponent->MoveSpeed = 400.0f;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Swimming);
		Movement->MaxSwimSpeed = StatComponent->MoveSpeed;
		Movement->BrakingDecelerationSwimming = 500.0f;
		Movement->GravityScale = 0.0f;
	}
	SprintMultiplier = 1.75f;

	// Debug용 카메라
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;

	bUseDebugCamera = false;
	ThirdPersonCameraComponent->SetActive(false);

	OxygenComponent = CreateDefaultSubobject<UOxygenComponent>(TEXT("OxygenComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	ShieldComponent = CreateDefaultSubobject<UShieldComponent>(TEXT("ShieldComponent"));

	InteractionComponent = CreateDefaultSubobject<UADInteractionComponent>(TEXT("InteractionComponent"));
	InteractableComponent = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComponent"));
	ShopInteractionComponent = CreateDefaultSubobject<UShopInteractionComponent>(TEXT("ShopInteractionComponent"));
	EquipUseComponent = CreateDefaultSubobject<UEquipUseComponent>(TEXT("EquipUseComponent"));

	LanternComponent = CreateDefaultSubobject<ULanternComponent>(TEXT("LanternComponent"));
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));
	
	bIsRadarOn = false;
	RadarOffset = FVector(150.0f, 0.0f, 0.0f);
	RadarRotation = FRotator(90.0f, 0.0f, 0.0f);

	EnvState = EEnvState::Underwater;
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetDebugCameraMode(bUseDebugCamera);
	
	RootComponent->PhysicsVolumeChangedDelegate.AddDynamic(this, &AUnderwaterCharacter::OnPhysicsVolumeChanged);

	StaminaComponent->OnSprintStateChanged.AddDynamic(this, &AUnderwaterCharacter::OnSprintStateChanged);
	OxygenComponent->OnOxygenLevelChanged.AddDynamic(this, &AUnderwaterCharacter::OnOxygenLevelChanged);
	OxygenComponent->OnOxygenDepleted.AddDynamic(this, &AUnderwaterCharacter::OnOxygenDepleted);

	StatComponent->OnHealthChanged.AddDynamic(this, &AUnderwaterCharacter::OnHealthChanged);
	StatComponent->OnMoveSpeedChanged.AddDynamic(this, &AUnderwaterCharacter::OnMoveSpeedChanged);
	
	NoiseEmitterComponent = NewObject<UPawnNoiseEmitterComponent>(this);
	NoiseEmitterComponent->RegisterComponent();

	// @ToDO: Controller 부분으로 분리
	if (IsLocallyControlled() && HoldWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		// 인스턴스 생성 → 뷰포트에 붙이지 않음(필요 시 Remove/Attach)
		HoldWidgetInstance = CreateWidget<UHoldInteractionWidget>(PC, HoldWidgetClass);
		
		InteractionComponent->OnHoldStart.AddDynamic(HoldWidgetInstance, &UHoldInteractionWidget::HandleHoldStart);
		InteractionComponent->OnHoldCancel.AddDynamic(HoldWidgetInstance, &UHoldInteractionWidget::HandleHoldCancel);
	}

	SpawnRadar();
	SpawnFlipperMesh();
}

void AUnderwaterCharacter::InitFromPlayerState(AADPlayerState* ADPlayerState)
{
	if (ADPlayerState == nullptr)
	{
		return;
	}
	
	if (UADInventoryComponent* Inventory = ADPlayerState->GetInventory())
	{
		CachedInventoryComponent = Inventory;
		// 인벤토리가 변경될 떄 OnRep에서도 호출되기 때문에 여기서 바인딩하면 Server, Client 양쪽에서 바인딩 가능하다.
		Inventory->InventoryUpdateDelegate.AddUObject(this, &AUnderwaterCharacter::AdjustSpeed);
	}
	else
	{
		LOGVN(Error, TEXT("Inventory Component Init failed : %d"), GetUniqueID());
	}

	if (UUpgradeComponent* Upgrade = ADPlayerState->GetUpgradeComp())
	{
		ApplyUpgradeFactor(Upgrade);
	}
	else
	{
		LOGVN(Error, TEXT("Upgrade Component Init failed : %d"), GetUniqueID());
	}

	// 리스폰 시에도 현재 무게에 따라 속도를 조정한다.
	AdjustSpeed();
}

void AUnderwaterCharacter::ApplyUpgradeFactor(UUpgradeComponent* UpgradeComponent)
{
	if (!IsValid(UpgradeComponent))
	{
		return;
	}

	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGVN(Error, TEXT("DataTableSubsystem is not valid"));
		return;
	}
	
	for (uint8 i = 0; i < static_cast<uint8>(EUpgradeType::Max); ++i)
	{
	    const EUpgradeType Type = static_cast<EUpgradeType>(i);
		
		const uint8 Grade = UpgradeComponent->GetCurrentGrade(Type);
		const FUpgradeDataRow* UpgradeData = DataTableSubsystem->GetUpgradeData(Type, Grade);
		if (UpgradeData == nullptr)
		{
			LOGVN(Error, TEXT("UpgradeData is not valid"));
			continue;
		}
		
		// Stat Factor는 정수형으로 저장되어 있다.
		const float StatFactor = UpgradeData->StatFactor;
		
	    switch (Type)
	    {
		    case EUpgradeType::Gather:
			    GatherMultiplier = StatFactor / 100.0f;
			    break;
	    	case EUpgradeType::Oxygen:
	    		OxygenComponent->InitOxygenSystem(StatFactor, StatFactor);
			    break;
	    	case EUpgradeType::Speed:
	    		// 최종 속도는 나중에 AdjustSpeed를 통해서 계산된다. 현재는 BaseSpeed만 조정하면 된다.
	    		StatComponent->MoveSpeed += StatFactor;
	    		break;
			case EUpgradeType::Light:
	    		// @TODO Apply Light Component Upgrade 
	    		break;
		    default: ;
	    		break;
	    }
	}
}

void AUnderwaterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AADPlayerState* ADPlayerState = GetPlayerState<AADPlayerState>())
	{
		InitFromPlayerState(ADPlayerState);
	}
	else
	{
		LOGVN(Error, TEXT("Player State Init failed : %d"), GetUniqueID());
	}
}

void AUnderwaterCharacter::PostNetInit()
{
	Super::PostNetInit();

	if (IsValid(EquipUseComponent))
	{
		EquipUseComponent->InitializeAmmoUI();
	}
}

void AUnderwaterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AADPlayerState* ADPlayerState = GetPlayerState<AADPlayerState>())
	{
		InitFromPlayerState(ADPlayerState);
	}
	else
	{
		LOGVN(Error, TEXT("Player State Init failed : %d"), GetUniqueID());
	}
}

void AUnderwaterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnderwaterCharacter, bIsRadarOn);
}

void AUnderwaterCharacter::SetEnvState(EEnvState State)
{
	if (EnvState == State)
	{
		return;
	}
	const EEnvState OldState = EnvState;
	EnvState = State;

	switch (EnvState)
	{
	case EEnvState::Underwater:
		GetCharacterMovement()->GravityScale = 0.0f;
		SetFlipperMeshVisibility(true);
		FirstPersonCameraArm->bEnableCameraRotationLag = true;
		Mesh1PSpringArm->bEnableCameraRotationLag = true;
		OxygenComponent->SetShouldConsumeOxygen(true);
		bCanUseEquipment = true;
		break;
	case EEnvState::Ground:
		// 지상에서는 이동 방향으로 회전을 하게 한다.
		GetCharacterMovement()->GravityScale = GetWorld()->GetGravityZ() / ExpectedGravityZ;
		SetFlipperMeshVisibility(false);
		FirstPersonCameraArm->bEnableCameraRotationLag = false;
		Mesh1PSpringArm->bEnableCameraRotationLag = false;
		OxygenComponent->SetShouldConsumeOxygen(false);
		bCanUseEquipment = false;
		break;
	default:
		UE_LOG(AbyssDiver, Error, TEXT("Invalid Character State"));
		break;
	}

	OnEnvStateChangedDelegate.Broadcast(OldState, EnvState);
	K2_OnEnvStateChanged(OldState, EnvState);
}

void AUnderwaterCharacter::StartCaptureState()
{
	if(bIsCaptured || !HasAuthority())
	{
		return;
	}

	bIsCaptured = true;
	M_StartCaptureState();
}

void AUnderwaterCharacter::StopCaptureState()
{
	// 사망 처리 시에도 StopCaptureState가 호출된다.
	// 즉, 사망이 확정되는 시점이 다르게 작동할 수 있다.
	
	if (!bIsCaptured || !HasAuthority())
	{
		return;
	}

	bIsCaptured = false;
	M_StopCaptureState();
}

void AUnderwaterCharacter::EmitBloodNoise()
{
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->MakeNoise(this, BloodEmitPower, GetActorLocation());
	}
}

void AUnderwaterCharacter::M_PlayMontageOnBothMesh_Implementation(UAnimMontage* Montage, float InPlayRate, FName StartSectionName, FAnimSyncState NewAnimSyncState)
{
	if (Montage == nullptr)
	{
		return;
	}

	bIsAnim3PSyncStateOverride = true;
	bIsAnim1PSyncStateOverride = true;
	OverrideAnimSyncState = NewAnimSyncState;
	
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		AnimInstance->Montage_Play(Montage, InPlayRate);
		if (StartSectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
		}
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(Montage, InPlayRate);
		if (StartSectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
		}
	}
}

void AUnderwaterCharacter::M_StopAllMontagesOnBothMesh_Implementation(float BlendOUt)
{
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(BlendOUt);
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(BlendOUt);
	}
}

void AUnderwaterCharacter::RequestChangeAnimSyncState(FAnimSyncState NewAnimSyncState)
{
	if (HasAuthority())
	{
		M_UpdateAnimSyncState(NewAnimSyncState);
	}
	else
	{
		S_ChangeAnimSyncState(NewAnimSyncState);
	}
}

void AUnderwaterCharacter::CleanupToolAndEffects()
{
	if (SpawnedTool1P || SpawnedTool3P)
	{
		SpawnedTool1P->Destroy();
		SpawnedTool1P = nullptr;

		SpawnedTool3P->Destroy();
		SpawnedTool3P = nullptr;
	}
}

void AUnderwaterCharacter::SpawnAndAttachTool(TSubclassOf<AActor> ToolClass)
{
	if (SpawnedTool1P || SpawnedTool3P || !ToolClass) return;
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	SpawnedTool1P = GetWorld()->SpawnActor<AActor>(
		ToolClass,
		GetActorLocation(),
		GetActorRotation(),
		Params
	);
	if (AADLaserCutter* Laser1P = Cast<AADLaserCutter>(SpawnedTool1P))
		Laser1P->M_SetupVisibility(true);

	SpawnedTool3P = GetWorld()->SpawnActor<AActor>(
		ToolClass,
		GetActorLocation(),
		GetActorRotation(),
		Params
	);

	if (AADLaserCutter* Laser3P = Cast<AADLaserCutter>(SpawnedTool3P))
		Laser3P->M_SetupVisibility(false);

	if (!SpawnedTool1P || !SpawnedTool3P) return;

	SpawnedTool1P->SetActorEnableCollision(false);
	SpawnedTool3P->SetActorEnableCollision(false);

	// 1-인칭
	SpawnedTool1P->AttachToComponent(
		GetMesh1P(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		LaserSocketName
	);


	// 3-인칭
	SpawnedTool3P->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		LaserSocketName
	);
	
}

void AUnderwaterCharacter::OnMoveSpeedChanged(float NewMoveSpeed)
{
	AdjustSpeed();
}

bool AUnderwaterCharacter::CanJumpInternal_Implementation() const
{
	// Landed와 마찮가지로 Server, 연관된 Client 모두에서 호출된다.

	// 착지 후 일정 시간 점프 입력을 방지한다.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastLandedTime < LandedJumpBlockTime)
	{
		return false;
	}
	
	return Super::CanJumpInternal_Implementation();
}

UStaticMeshComponent* AUnderwaterCharacter::CreateAndAttachMesh(const FString& ComponentName, UStaticMesh* MeshAsset, USceneComponent* Parent, FName SocketName, bool bIsThirdPerson)
{
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, *ComponentName);
	if (MeshComponent == nullptr)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("Failed to create StaticMeshComponent: %s"), *ComponentName);
		return nullptr;
	}

	MeshComponent->SetStaticMesh(MeshAsset);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetOwnerNoSee(bIsThirdPerson);
	MeshComponent->SetOnlyOwnerSee(!bIsThirdPerson);
	MeshComponent->CastShadow = bIsThirdPerson;
	MeshComponent->bCastHiddenShadow = bIsThirdPerson;
	MeshComponent->RegisterComponent();

	MeshComponent->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	MeshComponent->SetRelativeTransform(FTransform::Identity);

	return MeshComponent;
}

void AUnderwaterCharacter::SpawnFlipperMesh()
{
	if (LeftFlipperMesh == nullptr || RightFlipperMesh == nullptr)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Flipper Mesh is not set"));
		return;
	}

	// 모든 노드(Guest, Host)의 모든 캐릭터에서 3인칭 메시를 생성한다.
	// 다만, Owner No See 설정이기 때문에 Local Player는 보이지 않게 된다.
	LeftFlipperMesh3PComponent = CreateAndAttachMesh(
		TEXT("LeftFlipperMesh3P"),
		LeftFlipperMesh,
		GetMesh(),
		LeftFlipperSocketName,
		true
	);
	
	RightFlipperMesh3PComponent = CreateAndAttachMesh(
		TEXT("RightFlipperMesh3P"),
		RightFlipperMesh,
		GetMesh(),
		RightFlipperSocketName,
		true
	);

	// 1인칭 메시는 Local Player에서만 생성한다.
	if (IsLocallyControlled())
	{
		LeftFlipperMesh1PComponent = CreateAndAttachMesh(
			TEXT("LeftFlipperMesh1P"),
			LeftFlipperMesh,
			Mesh1P,
			LeftFlipperSocketName,
			false
		);
		
		RightFlipperMesh1PComponent = CreateAndAttachMesh(
			TEXT("RightFlipperMesh1P"),
			RightFlipperMesh,
			Mesh1P,
			RightFlipperSocketName,
			false
		);
	}
}

void AUnderwaterCharacter::SetFlipperMeshVisibility(const bool bVisible)
{
	if (LeftFlipperMesh1PComponent)
	{
		LeftFlipperMesh1PComponent->SetVisibility(bVisible);
	}
	if (RightFlipperMesh1PComponent)
	{
		RightFlipperMesh1PComponent->SetVisibility(bVisible);
	}

	// 3인칭 메시의 경우 bCastHiddenShadow 가 true로 설정되어 있기 떄문에 그림자 설정 자체를 꺼야 한다.
	if (LeftFlipperMesh3PComponent)
	{
		LeftFlipperMesh3PComponent->SetCastShadow(bVisible);
		LeftFlipperMesh3PComponent->SetHiddenInGame(!bVisible);
	}
	if (RightFlipperMesh3PComponent)
	{
		// RightFlipperMesh3PComponent->(bVisible);
		RightFlipperMesh3PComponent->SetCastShadow(bVisible);
		RightFlipperMesh3PComponent->SetHiddenInGame(!bVisible);
	}
}

void AUnderwaterCharacter::S_ChangeAnimSyncState_Implementation(FAnimSyncState NewAnimSyncState)
{
	RequestChangeAnimSyncState(NewAnimSyncState);
}

void AUnderwaterCharacter::M_UpdateAnimSyncState_Implementation(FAnimSyncState NewAnimSyncState)
{
	AnimSyncState = NewAnimSyncState;
}

void AUnderwaterCharacter::SetCharacterState(const ECharacterState NewCharacterState)
{
	if (!HasAuthority() || CharacterState == NewCharacterState)	
	{
		return;
	}

	LOGVN(Warning, TEXT("Character State Changed : %s"), *UEnum::GetDisplayValueAsText(NewCharacterState).ToString());

	// 각 로직을 Multicast에서 처리하도록 한다.
	// Listen Server Model 이므로 Multicast에서 Server 로직을 같이 처리할 수 있다.
	M_NotifyStateChange(NewCharacterState);
}

void AUnderwaterCharacter::M_NotifyStateChange_Implementation(ECharacterState NewCharacterState)
{
	HandleExitState(CharacterState);

	ECharacterState OldCharacterState = CharacterState;
	CharacterState = NewCharacterState;
	
	HandleEnterState(CharacterState);

	OnCharacterStateChangedDelegate.Broadcast(OldCharacterState, NewCharacterState);
}

void AUnderwaterCharacter::HandleEnterState(ECharacterState HandleCharacterState)
{
	if (HandleCharacterState == ECharacterState::Groggy)
	{
		HandleEnterGroggy();
	}
	else if (HandleCharacterState == ECharacterState::Normal)
	{
		HandleEnterNormal();
	}
	else if (HandleCharacterState == ECharacterState::Death)
	{
		HandleEnterDeath();
	}
	else
	{
		LOGVN(Error, TEXT("Invalid Character State"));
	}
}

void AUnderwaterCharacter::HandleExitState(ECharacterState HandleCharacterState)
{
	if (HandleCharacterState == ECharacterState::Groggy)
	{
		HandleExitGroggy();
	}
	else if (HandleCharacterState == ECharacterState::Normal)
	{
		HandleExitNormal();
	}
	else if (HandleCharacterState == ECharacterState::Death)
	{
		// Death 상황에서 전이되는 State는 없다.
	}
	else
	{
		LOGVN(Error, TEXT("Invalid Character State"));
	}
}

void AUnderwaterCharacter::HandleEnterGroggy()
{
	// Client에서는 남은 시간을 측정하기 위해 Timer를 설정한다.
	// 추후에 오차가 커질 경우 Timer를 사용하지 않고 시작 시간을 기점으로 계산하도록 한다.
	// Transition 4

	GetWorldTimerManager().SetTimer(GroggyTimer, FTimerDelegate::CreateUObject(this, &AUnderwaterCharacter::SetCharacterState, ECharacterState::Death), GroggyDuration, false);
	
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(true);
		}

		LookSensitivity = GroggyLookSensitivity;

		InteractionComponent->OnInteractReleased();
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
}

void AUnderwaterCharacter::HandleExitGroggy()
{
	GetWorldTimerManager().ClearTimer(GroggyTimer);
	// @TODO
	// 1. Groggy UI 제거
	// 2. Interaction 기능을 비활성화한다.
	GroggyDuration = CalculateGroggyTime(GroggyDuration, ++GroggyCount);
}

void AUnderwaterCharacter::HandleEnterNormal()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(false);
		}

		LookSensitivity = NormalLookSensitivity;
	}
}

void AUnderwaterCharacter::HandleExitNormal()
{
	// Stamina는 Replicate 되므로 Server에서 한 번만 정지하면 된다.
	if (HasAuthority())
	{
		StaminaComponent->RequestStopSprint();
	}

	// @TODO
	// 1. Stamina 회복 정지
	// 2. Character 체력 기능 활성화
}

void AUnderwaterCharacter::HandleEnterDeath()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);
		}

		InteractionComponent->OnInteractReleased();
	}

	// @TODO 사망 처리
	// 1. Server의 경우 Game Mode에 Report
	// 2. 사망 시의 UI 출력
	// 3. 사망 시의 캐릭터 처리 : 충돌 처리라던가 삭제 등

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
	
	K2_OnDeath();
	OnDeathDelegate.Broadcast();
}

void AUnderwaterCharacter::S_Revive_Implementation()
{
	RequestRevive();
}

float AUnderwaterCharacter::CalculateGroggyTime(float CurrentGroggyDuration, uint8 CalculateGroggyCount) const
{
	return FMath::Max(CurrentGroggyDuration * (1 - GroggyReductionRate), MinGroggyDuration);
}

void AUnderwaterCharacter::M_StartCaptureState_Implementation()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);

			PlayerController->PlayerCameraManager->StartCameraFade(
				0.0f,
				1.0f,
				CaptureFadeTime,
				FLinearColor::Black,
				false,
				true
			);
		}

		bCanUseEquipment = false;
		// Play SFX
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AUnderwaterCharacter::M_StopCaptureState_Implementation()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->ResetIgnoreLookInput();
			PlayerController->ResetIgnoreMoveInput();

			PlayerController->PlayerCameraManager->StartCameraFade(
				1.0f,
				0.0f,
				CaptureFadeTime,
				FLinearColor::Black,
				false,
				true
			);

			bCanUseEquipment = true;
		}
	}
	
	// Play SFX

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void AUnderwaterCharacter::AdjustSpeed()
{
	const float BaseSpeed = StaminaComponent->IsSprinting() ? StatComponent->MoveSpeed * SprintMultiplier : StatComponent->MoveSpeed;

	// 추후 Multiplier 종류가 늘어나면 Multiplier를 합산하도록 한다.
	float Multiplier = 1.0f;
	if (IsOverloaded())
	{
		Multiplier = 1 - OverloadSpeedFactor;
	}
	Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
	

	EffectiveSpeed = BaseSpeed * Multiplier;
	if (EnvState == EEnvState::Underwater)
	{
		GetCharacterMovement()->MaxSwimSpeed = EffectiveSpeed;
	}
	else if (EnvState == EEnvState::Ground)
	{
		GetCharacterMovement()->MaxWalkSpeed = EffectiveSpeed;
	}
	else
	{
		LOGVN(Error, TEXT("Invalid Character State"));
	}
}

void AUnderwaterCharacter::SpawnRadar()
{
	if (RadarClass == nullptr)
	{
		LOGVN(Error, TEXT("RadarClass is not valid"));
		return;
	}
	// Radar는 Local에서만 존재하면 된다.
	if (!IsLocallyControlled())
	{
		return;
	}

	FVector SpawnLocation = FirstPersonCameraComponent->GetComponentTransform().TransformPosition(RadarOffset);
	FRotator SpawnRotation = FirstPersonCameraComponent->GetComponentRotation() + RadarRotation;

	// @ToDO : Forward Actor에 맞추어서 Radar 회전
	RadarObject = GetWorld()->SpawnActor<ARadar>(RadarClass, SpawnLocation, SpawnRotation);
	RadarObject->AttachToComponent(FirstPersonCameraComponent, FAttachmentTransformRules::KeepWorldTransform);
	RadarObject->UpdateRadarSourceComponent(GetRootComponent(), GetRootComponent());
	SetRadarVisibility(false);
}

void AUnderwaterCharacter::RequestToggleRadar()
{
	if (HasAuthority())
	{
		bIsRadarOn = !bIsRadarOn;
		SetRadarVisibility(bIsRadarOn);
	}
	else
	{
		S_ToggleRadar();
	}
}

void AUnderwaterCharacter::SetRadarVisibility(bool bRadarVisible)
{
	if (!IsValid(RadarObject))
	{
		return;
	}
	
	// Visible == true 이면 Hidden == false이다.
	RadarObject->SetActorHiddenInGame(!bRadarVisible);
}

void AUnderwaterCharacter::S_ToggleRadar_Implementation()
{
	RequestToggleRadar();
}

void AUnderwaterCharacter::OnRep_bIsRadarOn()
{
	SetRadarVisibility(bIsRadarOn);
}

void AUnderwaterCharacter::OnOxygenLevelChanged(float CurrentOxygenLevel, float MaxOxygenLevel)
{
	// 산소 상태에 따라서 최대 스테미나를 조정한다.
	// 현재로서는 최대 산소량과 최대 스테미나량과 동일하고
	// 스테미나 소모를 초당 100으로 설정하고 있다.
	// 예를 들면 산소량 600이면 600초를 버틸 수 있고 최대 스테미나량이 600이 된다.
	// 스테미나 소모량이 100이므로 6초를 버틸 수 있다.

	// 헤더의 주석에 작성했지만 현재로는 간단하게 구현 위주로 작성한다.
	StaminaComponent->SetMaxStamina(CurrentOxygenLevel);
}


void AUnderwaterCharacter::OnOxygenDepleted()
{
	// Transition 2, 3
	SetCharacterState(ECharacterState::Death);
}

void AUnderwaterCharacter::OnHealthChanged(int32 CurrentHealth, int32 MaxHealth)
{
	if (HasAuthority() && CurrentHealth <= 0)
	{
		// Transition 1
		SetCharacterState(ECharacterState::Groggy);
	}
}

void AUnderwaterCharacter::OnPhysicsVolumeChanged(class APhysicsVolume* NewVolume) // Delegate에 const가 없기 떄문에 NewVolume 앞에 const를 붙이지 않는다.
{
	LOG_NETWORK(LogAbyssDiverCharacter, Display, TEXT("Physics Volume Changed : %s"), *NewVolume->GetName());

	const EEnvState NewEnvState = NewVolume->bWaterVolume ? EEnvState::Underwater : EEnvState::Ground;
	SetEnvState(NewEnvState);
}

void AUnderwaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(
				MoveAction, 
				ETriggerEvent::Started, 
				this, 
				&AUnderwaterCharacter::Move
			);
			EnhancedInput->BindAction(
				MoveAction, 
				ETriggerEvent::Triggered, 
				this, 
				&AUnderwaterCharacter::Move
			);
		}

		if (JumpAction)
		{
			EnhancedInput->BindAction(
				JumpAction,
				ETriggerEvent::Triggered,
				this,
				&AUnderwaterCharacter::JumpInputStart
			);
			EnhancedInput->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::JumpInputStop
			);
		}

		if (SprintAction)
		{
			EnhancedInput->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::StartSprint
			);
			EnhancedInput->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::StopSprint
			);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&AUnderwaterCharacter::Look
			);
		}

		if (FireAction)
		{
			EnhancedInput->BindAction(
				FireAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Fire
			);
			EnhancedInput->BindAction(
				FireAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::StopFire
			);
		}

		if (AimAction)
		{
			EnhancedInput->BindAction(
				AimAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Aim
			);
		}

		if (InteractionAction)
		{
			EnhancedInput->BindAction(
				InteractionAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Interaction
			);

			EnhancedInput->BindAction(
				InteractionAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::CompleteInteraction
			);
		}

		if (LightAction)
		{
			EnhancedInput->BindAction(
				LightAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Light
			);
		}

		if (RadarAction)
		{
			EnhancedInput->BindAction(
				RadarAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Radar
			);
		}

		if (ReloadAction)
		{
			EnhancedInput->BindAction(
				ReloadAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Reload
			);
			EnhancedInput->BindAction(
				ReloadAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::CompleteReload
			);
		}

		if (EquipSlot1Action)
		{
			EnhancedInput->BindAction(
				EquipSlot1Action,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::EquipSlot1
			);
		}

		if (EquipSlot2Action)
		{
			EnhancedInput->BindAction(
				EquipSlot2Action,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::EquipSlot2
			);
		}

		if (EquipSlot3Action)
		{
			EnhancedInput->BindAction(
				EquipSlot3Action,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::EquipSlot3
			);
		}
	}
	else
	{
		UE_LOG(AbyssDiver, Error, TEXT("Failed to find an Enhanced Input Component."))
	}
}

float AUnderwaterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	// 정해져야 할 것
	// 1. EmitBloodNoise를 Shield만 소모됬을 때 호출할 것인지

	const FShieldAbsorbResult ShieldAbsorbResult = ShieldComponent->AbsorbDamage(DamageAmount);
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Take Damage : %f, Remaining Damage : %f"), DamageAmount, ShieldAbsorbResult.RemainingDamage);

	const float ActualDamage = Super::TakeDamage(ShieldAbsorbResult.RemainingDamage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f)
	{
		EmitBloodNoise();
	}

	return ActualDamage;
}

void AUnderwaterCharacter::InteractHold_Implementation(AActor* InstigatorActor)
{
	LOGN(TEXT("Interact Hold : %s"), *GetName());
	if (!HasAuthority() || CharacterState != ECharacterState::Groggy)
	{
		return;
	}

	RequestRevive();
}

void AUnderwaterCharacter::OnHoldStart_Implementation(APawn* InstigatorPawn)
{
}

void AUnderwaterCharacter::OnHoldStop_Implementation(APawn* InstigatorPawn)
{
}

bool AUnderwaterCharacter::CanHighlight_Implementation() const
{
	return CharacterState == ECharacterState::Groggy;
}

float AUnderwaterCharacter::GetHoldDuration_Implementation() const
{
	return RescueRequireTime;
}

UADInteractableComponent* AUnderwaterCharacter::GetInteractableComponent() const
{
	return InteractableComponent;
}

bool AUnderwaterCharacter::IsHoldMode() const
{
	return  true;
}

void AUnderwaterCharacter::RequestRevive()
{
	if (HasAuthority())
	{
		// Transition 5
		if (CharacterState != ECharacterState::Groggy)
		{
			return;
		}

		SetCharacterState(ECharacterState::Normal);
		const float RecoveryHealth = StatComponent->GetMaxHealth() * RecoveryHealthPercentage;
		StatComponent->RestoreHealth(RecoveryHealth);
	}
	else
	{
		S_Revive();
	}
}

void AUnderwaterCharacter::Move(const FInputActionValue& InputActionValue)
{
	// To-Do
	// Can Move 확인
	
	// 캐릭터의 XYZ 축을 기준으로 입력을 받는다.
	const FVector MoveInput = InputActionValue.Get<FVector>();

	if (EnvState == EEnvState::Ground)
	{
		MoveGround(MoveInput);
	}
	else if (EnvState == EEnvState::Underwater)
	{
		MoveUnderwater(MoveInput);
	}
}

void AUnderwaterCharacter::MoveUnderwater(const FVector MoveInput)
{
	// Forward : Camera Forward with pitch
	const FRotator ControlRotation = GetControlRotation();
	const FVector ForwardVector = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);

	// Right : Camera Right without pitch
	const FRotator XYRotation = FRotator(0.f, ControlRotation.Yaw, 0.f);
	const FVector RightVector = FRotationMatrix(XYRotation).GetUnitAxis(EAxis::Y);

	if (!FMath::IsNearlyZero(MoveInput.X)) 
	{
		AddMovementInput(ForwardVector, MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightVector, MoveInput.Y);
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddMovementInput(FVector::UpVector, MoveInput.Z);
	}
}

void AUnderwaterCharacter::MoveGround(FVector MoveInput)
{
	const FRotator ControllerRotator = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);

	// World에서 Controller의 X축, Y축 방향
	const FVector ForwardVector = FRotationMatrix(ControllerRotator).GetUnitAxis(EAxis::X);
	const FVector RightVector = FRotationMatrix(ControllerRotator).GetUnitAxis(EAxis::Y);

	// 전후 이동
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ForwardVector, MoveInput.X);
	}
	// 좌우 이동
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightVector, MoveInput.Y);
	}
}

void AUnderwaterCharacter::JumpInputStart(const FInputActionValue& InputActionValue)
{
	if (EnvState == EEnvState::Underwater)	
	{
		// 수중에서는 점프가 불가능하다.
		return;
	}

	Jump();
}

void AUnderwaterCharacter::JumpInputStop(const FInputActionValue& InputActionValue)
{
	if (EnvState == EEnvState::Underwater)
	{
		// 수중에서는 점프가 불가능하다.
		return;
	}

	StopJumping();
}

void AUnderwaterCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	LocomotionMode = ELocomotionMode::Jumping;
}

void AUnderwaterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Landed 호출 시점
	// Host : Host만 호출
	// Client : Host와 Client 모두 호출
	// 현재로는 Jump, Landed를 이용해서 애니메이션을 설정할 수 없다.

	LastLandedTime = GetWorld()->GetTimeSeconds();
	LocomotionMode = ELocomotionMode::None;
}

void AUnderwaterCharacter::StartSprint(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}
	StaminaComponent->RequestStartSprint();
}

void AUnderwaterCharacter::StopSprint(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}
	StaminaComponent->RequestStopSprint();
}

void AUnderwaterCharacter::OnSprintStateChanged(bool bNewSprinting)
{
	AdjustSpeed();
}

void AUnderwaterCharacter::Look(const FInputActionValue& InputActionValue)
{
	FVector2d LookInput = InputActionValue.Get<FVector2d>();

	// Y축은 반전되어서 들어오므로 그대로 적용한다.
	AddControllerYawInput(LookInput.X * LookSensitivity);
	AddControllerPitchInput(LookInput.Y * LookSensitivity);
}

void AUnderwaterCharacter::Fire(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	EquipUseComponent->HandleLeftClick();
}

void AUnderwaterCharacter::StopFire(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	EquipUseComponent->HandleLeftRelease();
}

void AUnderwaterCharacter::Reload(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	EquipUseComponent->HandleRKey();
}

void AUnderwaterCharacter::CompleteReload(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	EquipUseComponent->HandleRKeyRelease();
}

void AUnderwaterCharacter::Aim(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Interaction(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}
	InteractionComponent->OnInteractPressed();
}

void AUnderwaterCharacter::CompleteInteraction(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}
	InteractionComponent->OnInteractReleased();
}

void AUnderwaterCharacter::Light(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}

	// @TODO : Lantern Light 몬스터 인지 기능 추가
	LanternComponent->RequestToggleLanternLight();
}

void AUnderwaterCharacter::Radar(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal)
	{
		return;
	}
	RequestToggleRadar();
}

void AUnderwaterCharacter::EquipSlot1(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	CachedInventoryComponent->S_UseInventoryItem(EItemType::Equipment, 0);
}

void AUnderwaterCharacter::EquipSlot2(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	CachedInventoryComponent->S_UseInventoryItem(EItemType::Equipment, 1);
}

void AUnderwaterCharacter::EquipSlot3(const FInputActionValue& InputActionValue)
{
	if (CharacterState != ECharacterState::Normal || !bCanUseEquipment)
	{
		return;
	}
	CachedInventoryComponent->S_UseInventoryItem(EItemType::Equipment, 2);
}

void AUnderwaterCharacter::SetDebugCameraMode(bool bDebugCameraEnable)
{
	bUseDebugCamera = bDebugCameraEnable;
	if (bUseDebugCamera)
	{
		FirstPersonCameraComponent->SetActive(false);
		ThirdPersonCameraComponent->SetActive(true);
		GetMesh()->SetOwnerNoSee(false);
	}
	else
	{
		FirstPersonCameraComponent->SetActive(true);
		ThirdPersonCameraComponent->SetActive(false);
		GetMesh()->SetOwnerNoSee(true);
	}
}

void AUnderwaterCharacter::ToggleDebugCameraMode()
{
	bUseDebugCamera = !bUseDebugCamera;
	SetDebugCameraMode(bUseDebugCamera);
}

void AUnderwaterCharacter::OnMesh1PMontageStarted(UAnimMontage* Montage)
{
	OnMesh1PMontageStartedDelegate.Broadcast(Montage);
}

void AUnderwaterCharacter::OnMesh1PMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAnim1PSyncStateOverride = false;
	OnMesh1PMontageEndDelegate.Broadcast(Montage, bInterrupted);
}

void AUnderwaterCharacter::OnMesh3PMontageStarted(UAnimMontage* Montage)
{
	OnMesh3PMontageStartedDelegate.Broadcast(Montage);
}

void AUnderwaterCharacter::OnMesh3PMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAnim3PSyncStateOverride = false;
	OnMesh3PMontageEndDelegate.Broadcast(Montage, bInterrupted);
}

void AUnderwaterCharacter::SetupMontageCallbacks()
{
	if (UAnimInstance* AnimInstance3P = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance3P->OnMontageStarted.AddDynamic(this, &AUnderwaterCharacter::OnMesh3PMontageStarted);
		AnimInstance3P->OnMontageEnded.AddDynamic(this, &AUnderwaterCharacter::OnMesh3PMontageEnded);
	}

	if (UAnimInstance* AnimInstance1P = Mesh1P ? Mesh1P->GetAnimInstance() : nullptr)
	{
		AnimInstance1P->OnMontageStarted.AddDynamic(this, &AUnderwaterCharacter::OnMesh1PMontageStarted);
		AnimInstance1P->OnMontageEnded.AddDynamic(this, &AUnderwaterCharacter::OnMesh1PMontageEnded);
	}
}

bool AUnderwaterCharacter::IsAlive() const
{
	return CharacterState != ECharacterState::Death;
}

float AUnderwaterCharacter::GetRemainGroggyTime() const
{
	if (GetWorldTimerManager().IsTimerActive(GroggyTimer))
	{
		return GetWorldTimerManager().GetTimerRemaining(GroggyTimer);
	}
	return 0.0f;
}

bool AUnderwaterCharacter::IsSprinting() const
{
	return StaminaComponent->IsSprinting();
}

void AUnderwaterCharacter::SetHideInSeaweed(const bool bNewHideInSeaweed)
{
	bIsHideInSeaweed = bNewHideInSeaweed;
}

bool AUnderwaterCharacter::IsOverloaded() const
{
	return IsValid(CachedInventoryComponent) && CachedInventoryComponent->GetTotalWeight() >= OverloadWeight;
}
