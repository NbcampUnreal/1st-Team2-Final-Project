// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterCharacter.h"

#include "AbyssDiverUnderWorld.h"
#include "CableBindingActor.h"
#include "EnhancedInputComponent.h"
#include "LocomotionMode.h"
#include "PlayerComponent/OxygenComponent.h"
#include "PlayerComponent/StaminaComponent.h"
#include "StatComponent.h"
#include "UpgradeComponent.h"
#include "AbyssDiverUnderWorld/Interactable/Item/Component/ADInteractionComponent.h"
#include "Boss/Effect/PostProcessSettingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "Footstep/FootstepComponent.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADInGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interactable/Item/Component/EquipUseComponent.h"
#include "Interactable/OtherActors/Radars/Radar.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shops/ShopInteractionComponent.h"
#include "Subsystems/DataTableSubsystem.h"
#include "UI/HoldInteractionWidget.h"
#include "PlayerComponent/LanternComponent.h"
#include "PlayerComponent/ShieldComponent.h"
#include "PlayerComponent/UnderwaterEffectComponent.h"
#include "Interactable/EquipableComponent/EquipRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerComponent/CombatEffectComponent.h"
#include "PlayerComponent/NameWidgetComponent.h"
#include "PlayerComponent/RagdollReplicationComponent.h"

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

	bIsInvincible = false;

	bCanUseEquipment = true;
	
	bPlayingEmote = false;
	PlayEmoteIndex = INDEX_NONE;
	CameraTransitionUpdateInterval = 0.01f;
	CameraTransitionDirection = 1.0f;
	CameraTransitionTimeElapsed = 0.0f;
	CameraTransitionDuration = 0.25f;
	EmoteCameraTransitionLength = 500.0f;
	EmoteCameraTransitionEasingType = EEasingFunc::EaseInOut;

	LanternLength = 3000.0f;
	
	LeftFlipperSocketName = TEXT("foot_l_flipper_socket");
	RightFlipperSocketName = TEXT("foot_r_flipper_socket");
	
	LookSensitivity = 1.0f;
	NormalLookSensitivity = 1.0f;
	NormalStateFadeInDuration = 1.0f;
	
	GroggyDuration = 60.0f;
	GroggyReductionRate = 0.1f;
	MinGroggyDuration = 10.0f;
	GroggyCount = 0;
	GroggyLookSensitivity = 0.25f;
	RescueRequireTime = 6.0f;
	GroggyHitPenalty = 5.0f;
	RecoveryHealthPercentage = 0.1f;
	RecoveryOxygenPenaltyRate = 0.5f;

	bIsInCombat = false;
	TargetingActorCount = 0;
	HealthRegenDelay = 5.0f;
	HealthRegenRate = 0.1f;
	
	GatherMultiplier = 1.0f;
	
	bIsCaptured = false;
	CaptureFadeTime = 0.5f;

	bIsHideInSeaweed = false;
	
	BloodEmitPower = 1.0f;

	OverloadWeight = 40.0f;
	OverloadSpeedFactor = 0.4f;
	MinSpeed = 150.0f;
	BaseGroundSpeed = 600.0f;
	
	StatComponent->MoveSpeed = 400.0f;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Swimming);
		Movement->MaxSwimSpeed = StatComponent->MoveSpeed;
		Movement->BrakingDecelerationSwimming = 500.0f;
		Movement->GravityScale = 0.0f;
	}
	SprintMultiplier = 1.75f;
	ZoneSpeedMultiplier = 1.0f;

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

	UnderwaterEffectComponent = CreateDefaultSubobject<UUnderwaterEffectComponent>(TEXT("UnderwaterEffectComponent"));
	CombatEffectComponent = CreateDefaultSubobject<UCombatEffectComponent>(TEXT("CombatEffectComponent"));
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));

	RagdollComponent = CreateDefaultSubobject<URagdollReplicationComponent>(TEXT("RagdollComponent"));
	
	bIsRadarOn = false;
	RadarOffset = FVector(150.0f, 0.0f, 0.0f);
	RadarRotation = FRotator(90.0f, 0.0f, 0.0f);

	EnvironmentState = EEnvironmentState::Underwater;

	RadarReturnComponent->FactionTags.Init(TEXT("Friendly"), 1);
	EquipRenderComp = CreateDefaultSubobject<UEquipRenderComponent>(TEXT("EquipRenderComponent"));
	NameWidgetComponent = CreateDefaultSubobject<UNameWidgetComponent>(TEXT("NameWidgetComponent"));
	NameWidgetComponent->SetupAttachment(GetCapsuleComponent());
	NameWidgetComponent->SetRelativeLocation(FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f);

	InteractionDescription = TEXT("");
	bIsInteractionHoldMode = true;
	GroggyInteractionDescription = TEXT("Revive Character!");
	DeathGrabDescription = TEXT("Grab Character!");
	DeathGrabReleaseDescription = TEXT("Release Character!");

	BindMultiplier = 0.15f;

	bIsAttackedByEyeStalker = false;

	PostProcessSettingComponent = CreateDefaultSubobject<UPostProcessSettingComponent>(TEXT("PostProcessSettingComponent"));

	GetMesh()->SetLightingChannels(false, true, true);

	ResurrectSFX = ESFX::Resurrection;
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("[%s] Begin Play / Controller : %s / Locally Controlled : %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*GetNameSafe(GetController()),
		IsLocallyControlled() ? TEXT("Yes") : TEXT("No")
	);
	
	SetDebugCameraMode(bUseDebugCamera);
	
	RootComponent->PhysicsVolumeChangedDelegate.AddDynamic(this, &AUnderwaterCharacter::OnPhysicsVolumeChanged);

	StaminaComponent->OnSprintStateChanged.AddDynamic(this, &AUnderwaterCharacter::OnSprintStateChanged);
	OxygenComponent->OnOxygenLevelChanged.AddDynamic(this, &AUnderwaterCharacter::OnOxygenLevelChanged);
	OxygenComponent->OnOxygenDepleted.AddDynamic(this, &AUnderwaterCharacter::OnOxygenDepleted);

	StatComponent->OnHealthChanged.AddDynamic(this, &AUnderwaterCharacter::OnHealthChanged);
	StatComponent->OnMoveSpeedChanged.AddDynamic(this, &AUnderwaterCharacter::OnMoveSpeedChanged);
	
	NoiseEmitterComponent = NewObject<UPawnNoiseEmitterComponent>(this);
	NoiseEmitterComponent->RegisterComponent();

	SpawnRadar();
	SpawnFlipperMesh();
	LanternComponent->SpawnLight(GetMesh1PSpringArm(), LanternLength);

	if (HasAuthority())
	{
		if (AADInGameMode* GameManager = Cast<AADInGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GameManager->BindDelegate(this);
		}
	}

	// @ToDo : Enter Normal로 처리가 가능한 부분은 Enter Normal로 처리
	InteractableComponent->SetInteractable(false);
	AdjustSpeed();
}

void AUnderwaterCharacter::InitFromPlayerState(AADPlayerState* ADPlayerState)
{
	if (ADPlayerState == nullptr)
	{
		return;
	}

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("[%s] InitFromPlayerState/ NickName : %s / PlayerIndex : %d"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"), *ADPlayerState->GetPlayerNickname(), ADPlayerState->GetPlayerIndex());

	// @ToDo: 패키징에서 데이터를 제대로 받지 못하는 문제가 있다. 패키징하기 전에 수정할 것
	PlayerIndex = ADPlayerState->GetPlayerIndex();
	
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
		
		const int StatFactor = UpgradeData->StatFactor;
		
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
	    		BaseSwimSpeed += StatFactor;
	    		break;
			case EUpgradeType::Light:
	    		if (Grade > 1)
	    		{
	    			// 정수 곱하기 연산을 먼저하고 나누기 연산을 나중에 해서 소수점 오차를 줄인다.
	    			LanternLength = LanternLength * (100 + StatFactor) / 100.0f;
	    			LanternComponent->SetLightLength(LanternLength);
	    		}
	    		break;
		    default: ;
	    		break;
	    }
	}
}

void AUnderwaterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("[%s] PossessedBy / Controller : %s / Locally Controlled : %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*GetNameSafe(GetController()),
		IsLocallyControlled() ? TEXT("Yes") : TEXT("No")
	);

	OwnerController = Cast<AADPlayerController>(NewController);
	
	if (IsLocallyControlled())
	{
		SetMeshFirstPersonSetting(true);
	}
	
	if (AADPlayerState* ADPlayerState = GetPlayerState<AADPlayerState>())
	{
		InitFromPlayerState(ADPlayerState);
		if (!IsLocallyControlled())
		{
			NameWidgetComponent->SetNameText(ADPlayerState->GetPlayerNickname());
			NameWidgetComponent->SetEnable(true);
			UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Set Player Nick Name On Possess : %s"), *ADPlayerState->GetPlayerNickname());
		}

		// Possess는 Authority 상황에서 호출되므로 Server 로직만 작성하면 된다.
		if (ADPlayerState->HasBeenDead())
		{
			UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Player has been dead, Respawning..."));
			Respawn();	
		}
	}
	else
	{
		LOGVN(Error, TEXT("Player State Init failed : %d"), GetUniqueID());
	}
}

void AUnderwaterCharacter::UnPossessed()
{
	Super::UnPossessed();

	OnLeavePawn();
}

void AUnderwaterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	BaseSwimSpeed = StatComponent->MoveSpeed;	
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

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("[%s] OnRep_PlayerState / Controller : %s / Locally Controlled : %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*GetNameSafe(GetController()),
		IsLocallyControlled() ? TEXT("Yes") : TEXT("No")
	);
	
	// UnPossess 상황에서 Error 로그가 발생하지 않도록 수정
	if (APlayerState* CurrentPlayerState = GetPlayerState<AADPlayerState>())
	{
		if (AADPlayerState* ADPlayerState = Cast<AADPlayerState>(CurrentPlayerState))
		{
			InitFromPlayerState(ADPlayerState);
			if (!IsLocallyControlled())
			{
				NameWidgetComponent->SetNameText(ADPlayerState->GetPlayerNickname());
				NameWidgetComponent->SetEnable(true);
				UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Set Player Nick Name On Rep : %s"), *ADPlayerState->GetPlayerNickname());
			}
		}
		else
		{
			LOGVN(Error, TEXT("Player State Init failed : %d"), GetUniqueID());
		}
		if (IsLocallyControlled())
		{
			SetMeshFirstPersonSetting(true);
		}
	}
	else
	{
		OnLeavePawn();
	}
}

void AUnderwaterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnderwaterCharacter, bIsRadarOn);
	DOREPLIFETIME(AUnderwaterCharacter, CurrentTool);
	DOREPLIFETIME(AUnderwaterCharacter, BindCharacter);
	DOREPLIFETIME(AUnderwaterCharacter, BoundCharacters);
	DOREPLIFETIME(AUnderwaterCharacter, bIsInCombat);
}

void AUnderwaterCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UE_LOG(LogAbyssDiverCharacter,Display, TEXT("Movement Changed : %s"), *UEnum::GetValueAsString(GetCharacterMovement()->MovementMode));

	if (EnvironmentState != EEnvironmentState::Underwater)
	{
		return;
	}

	// Launch되어서 Falling으로 변경되고 Launch가 끝나서 Swimming으로 변경될 경우 Knockback이 종료된다.
	if (PrevMovementMode == MOVE_Falling && GetCharacterMovement()->MovementMode == MOVE_Swimming)
	{
		LOGVN(Display, TEXT("Knockback Ended: %s"), *GetName());
		OnKnockbackEndDelegate.Broadcast();
	}
}

void AUnderwaterCharacter::Destroyed()
{
	Super::Destroyed();

	DisconnectRope();
}

void AUnderwaterCharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	// LaunchCharacter는 Server, Client 양쪽에서 호출되어서 따로 Replicate할 필요는 없다.
	Super::LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);
	
	OnKnockbackDelegate.Broadcast(LaunchVelocity);
}

void AUnderwaterCharacter::SetEnvironmentState(EEnvironmentState State)
{
	if (EnvironmentState == State)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("EnvironmentState is already set to %s"), *UEnum::GetValueAsString(State));
		return;
	}
	const EEnvironmentState OldState = EnvironmentState;
	EnvironmentState = State;

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Environment State : %s -> %s"),
		*UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(EnvironmentState));
	
	switch (EnvironmentState)
	{
	case EEnvironmentState::Underwater:
		GetCharacterMovement()->GravityScale = 0.0f;
		SetFlipperMeshVisibility(true);
		FirstPersonCameraArm->bEnableCameraRotationLag = true;
		Mesh1PSpringArm->bEnableCameraRotationLag = true;
		OxygenComponent->SetShouldConsumeOxygen(true);
		bCanUseEquipment = true;
		break;
	case EEnvironmentState::Ground:
		GetCharacterMovement()->GravityScale = ExpectedGravityZ / GetWorld()->GetGravityZ();
		SetFlipperMeshVisibility(false);
		FirstPersonCameraArm->bEnableCameraRotationLag = false;
		Mesh1PSpringArm->bEnableCameraRotationLag = false;
		OxygenComponent->SetShouldConsumeOxygen(false);
		bCanUseEquipment = false;
		UpdateBlurEffect();
		if (AADPlayerState* ADPlayerState = GetPlayerState<AADPlayerState>())
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
		break;
	default:
		UE_LOG(AbyssDiver, Error, TEXT("Invalid Character State"));
		break;
	}

	AdjustSpeed();
	UpdateBlurEffect();
	
	OnEnvironmentStateChangedDelegate.Broadcast(OldState, EnvironmentState);
	K2_OnEnvironmentStateChanged(OldState, EnvironmentState);
}

void AUnderwaterCharacter::OnTargeted()
{
	if (HasAuthority())
	{
		TargetingActorCount++;
		StartCombat();
	}
}

void AUnderwaterCharacter::OnUntargeted()
{
	if (HasAuthority())
	{
		TargetingActorCount--;
		if (TargetingActorCount <= 0)
		{
			TargetingActorCount = 0;
			EndCombat();
		}
	}
}

void AUnderwaterCharacter::StartCaptureState()
{
	if (bIsCaptured || !HasAuthority() || CharacterState != ECharacterState::Normal)
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

void AUnderwaterCharacter::RequestBind(AUnderwaterCharacter* RequestBinderCharacter)
{
	LOGVN(Display, TEXT("Character %s bind to %s"), *GetName(), *RequestBinderCharacter->GetName());
	
	if (!HasAuthority() || RequestBinderCharacter == nullptr || CharacterState != ECharacterState::Death)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("RequestBind called in invalid state or not authority: %s"), *GetName());
		return;
	}
	
	if (BindCharacter == RequestBinderCharacter)
	{
		UnBind();
	}
	else
	{
		BindCharacter = RequestBinderCharacter;
		RequestBinderCharacter->BindToCharacter(this);
		
		ConnectRope(BindCharacter);

		UpdateBindInteractable();
		AdjustSpeed();
	}
}

void AUnderwaterCharacter::UnBind()
{
	if (!HasAuthority() || CharacterState != ECharacterState::Death)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("UnBind called in invalid state or not authority: %s"), *GetName());
		return;
	}

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Character %s unbind from %s"), *GetName(), BindCharacter ? *BindCharacter->GetName() : TEXT("None"));

	if (IsValid(BindCharacter))
	{
		BindCharacter->UnbindToCharacter(this);
		BindCharacter = nullptr;
	}

	DisconnectRope();
	UpdateBindInteractable();
}

void AUnderwaterCharacter::UnbindAllBoundCharacters()
{
	if (!HasAuthority() || CharacterState != ECharacterState::Normal)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("UnbindAllBoundCharacters called in invalid state or not authority: %s"), *GetName());
		return;
	}
	
	// Flow
	// - Binder Character : UnbindAllBoundCharacters
	// -- Bound Character -> UnBind
	// - Bound Character
	// -- BindCharacter -> UnbindToCharacter
	// --- BindCharacter::UnbindToCharacter : Remove Bound Characters

	// UnBind는 내부적으로 BoundCharacters를 수정하므로 복사본을 이용해서 순회한다.
	for (AUnderwaterCharacter* BoundCharacter : GetBoundCharacters())
	{
		if (IsValid(BoundCharacter))
		{
			BoundCharacter->UnBind();
		}
	}
}

void AUnderwaterCharacter::EmitBloodNoise()
{
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->MakeNoise(this, BloodEmitPower, GetActorLocation());
	}
}

void AUnderwaterCharacter::RequestPlayMontage(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage,
	float InPlayRate, FName StartSectionName)
{
	if (HasAuthority())
	{
		M_BroadcastPlayMontage(Mesh1PMontage, Mesh3PMontage, InPlayRate, StartSectionName);
	}
	else
	{
		S_PlayMontage(Mesh1PMontage, Mesh3PMontage, InPlayRate, StartSectionName);
	}
}

void AUnderwaterCharacter::S_PlayMontage_Implementation(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate, FName StartSectionName)
{
	RequestPlayMontage(Mesh1PMontage, Mesh3PMontage, InPlayRate, StartSectionName);
}

void AUnderwaterCharacter::M_BroadcastPlayMontage_Implementation(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate, FName StartSectionName)
{
	if (Mesh1PMontage)
	{
		if (GetMesh1P() && GetMesh1P()->GetAnimInstance())
		{
			GetMesh1P()->GetAnimInstance()->Montage_Play(Mesh1PMontage, InPlayRate);
			if (StartSectionName != NAME_None)
			{
				GetMesh1P()->GetAnimInstance()->Montage_JumpToSection(StartSectionName, Mesh1PMontage);
			}
		}
		else
		{
			LOGVN(Error, TEXT("Mesh1P AnimInstance is not valid: %s"), *GetName());
		}
	}
	if (Mesh3PMontage)
	{
		if (GetMesh() && GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(Mesh3PMontage, InPlayRate);
			if (StartSectionName != NAME_None)
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(StartSectionName, Mesh3PMontage);
			}
		}
		else
		{
			LOGVN(Error, TEXT("Mesh AnimInstance is not valid: %s"), *GetName());
		}
	}
}

void AUnderwaterCharacter::RequestStopAllMontage(EPlayAnimationTarget Target, float BlendOut)
{
	if (HasAuthority())
	{
		M_StopAllMontage(Target, BlendOut);
	}
	else
	{
		S_StopAllMontage(Target, BlendOut);
	}
}

void AUnderwaterCharacter::M_StopAllMontage_Implementation(EPlayAnimationTarget Target, float BlendOut)
{
	if (GetMesh1P() && GetMesh1P()->GetAnimInstance())
	{
		GetMesh1P()->GetAnimInstance()->StopAllMontages(BlendOut);
	}
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(BlendOut);
	}
}

void AUnderwaterCharacter::S_StopAllMontage_Implementation(EPlayAnimationTarget Target, float BlendOut)
{
	RequestStopAllMontage(Target, BlendOut);
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

void AUnderwaterCharacter::M_StopAllMontagesOnBothMesh_Implementation(float BlendOut)
{
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(BlendOut);
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(BlendOut);
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
	if (SpawnedTool)
	{
		EquipRenderComp->DetachItem(SpawnedTool);
		SpawnedTool->Destroy();
		CurrentTool->Destroy();
		SpawnedTool = nullptr;
		CurrentTool = nullptr;
	}
}

void AUnderwaterCharacter::SpawnAndAttachTool(TSubclassOf<AActor> ToolClass)
{
	if (SpawnedTool || !ToolClass || !HasAuthority()) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	SpawnedTool = GetWorld()->SpawnActor<AActor>(
		ToolClass,
		GetActorLocation(),
		GetActorRotation(),
		Params
	);
	SpawnedTool->SetActorEnableCollision(false);

	CurrentTool = SpawnedTool;
	OnRep_CurrentTool();
}

void AUnderwaterCharacter::OnRep_CurrentTool()
{
	LOG(TEXT("OnRep_CurrentTool for %s  EquipRenderComp=%s  IsRegistered=%d"),
		*GetNameSafe(this),
		*GetNameSafe(EquipRenderComp),
		EquipRenderComp ? EquipRenderComp->IsRegistered() : 0);

	if (PrevTool)
	{
		EquipRenderComp->DetachItem(PrevTool);
		PrevTool = nullptr;
		LOG(TEXT("PrevTool"));
	}

	if (CurrentTool)
	{
		if (USkeletalMeshComponent* Src = CurrentTool->FindComponentByClass<USkeletalMeshComponent>())
		{
			Src->SetVisibility(false, true);
			LOG(TEXT("Src is CurrentTool's SkeletalMesh"));
		}
		EquipRenderComp->AttachItem(CurrentTool, LaserSocketName);
		PrevTool = CurrentTool;                   // 다음 Detach 대비
		LOG(TEXT("CurrentTool's Owner : %s"), *CurrentTool->GetOwner()->GetName());
	}
}


void AUnderwaterCharacter::OnRep_InCombat()
{
	LOG(TEXT("OnRep_InCombat! %d"), bIsInCombat);
	if (UnderwaterEffectComponent)
	{
		if (bIsInCombat)
		{
			UnderwaterEffectComponent->StartCombatEffect();
		}
		else
		{
			UnderwaterEffectComponent->StopCombatEffect();
		}	
	}
}

void AUnderwaterCharacter::OnSpectated()
{
	SetMeshFirstPersonSetting(true);
}

void AUnderwaterCharacter::OnEndSpectated()
{
	SetMeshFirstPersonSetting(false);
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
	// 3인칭 메시 : Owner No See == true, Only Owner See == false, Owner는 볼 수 없고 다른 사람만 볼 수 있다.
	// 1인칭 메시 : Owner No See == false, Only Owner See == true, Owner만 볼 수 있고 다른 사람은 볼 수 없다.
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
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Character State Changed : %s -> %s | Authority : %s"),
		*UEnum::GetValueAsString(CharacterState),
		*UEnum::GetValueAsString(NewCharacterState),
		HasAuthority() ? TEXT("True") : TEXT("False")
	);
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

	GetWorldTimerManager().SetTimer(GroggyTimer,
	                                FTimerDelegate::CreateUObject(this, &AUnderwaterCharacter::SetCharacterState,
	                                ECharacterState::Death),
									GroggyDuration,
									false);

	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->ClientSetCameraFade(false);
			PlayerController->ClientSetCameraFade(true,
				FColor::Black,
				FVector2D(0.0f, 1.0f),
				GroggyDuration,
				true,
				true
			);
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
	
	InteractableComponent->SetInteractable(true);
	InteractionDescription = GroggyInteractionDescription;
	bIsInteractionHoldMode = true;
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
	if (HasAuthority())
	{
		if (StatComponent->GetCurrentHealth() <= StatComponent->GetMaxHealth())
		{
			StartHealthRegen();
		}
	}
	
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->ClientSetCameraFade(true,
				FColor::Black,
				FVector2D(-1.0f, 0.0f),
				NormalStateFadeInDuration,
				true,
				true
			);
		}

		LookSensitivity = NormalLookSensitivity;
	}
	
	InteractableComponent->SetInteractable(false);
}

void AUnderwaterCharacter::HandleExitNormal()
{
	// Stamina는 Replicate 되므로 Server에서 한 번만 정지하면 된다.
	if (HasAuthority())
	{
		StaminaComponent->RequestStopSprint();
		StopHealthRegen();

		UnbindAllBoundCharacters();
	}
}

void AUnderwaterCharacter::HandleEnterDeath()
{
	// Case1. Health == 0 -> Groggy -> Death : 이미 Black Out이 완료되었으므로 바로 Death를 종료
	// Case2. Normal -> Death : 산소가 없어서 사망, Black Out을 적용해서 Death 종료
	// Case3. Groggy -> Death | Oxygen == 0 : Groggy 상태에서 산소가 없어서 사망, Black Out을 적용해서 Death 종료

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DeathTimer,
			this,
			&AUnderwaterCharacter::EndDeath,
			DeathTransitionTime,
			false
		);
	}
	
	if (IsLocallyControlled())
	{
		// @ToDo: Death UI 출력
		// @ToDo: Death Camera Transition
		
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->ClientSetCameraFade(
				true, 
				FColor::Black,
				FVector2D(-1.0f, 1.0f),
				DeathTransitionTime,
				true,
				true
			);
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
	
	InteractableComponent->SetInteractable(true);
	InteractionDescription = DeathGrabDescription;
	bIsInteractionHoldMode = false;

	RagdollComponent->SetRagdollEnabled(true);
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
	if (HasAuthority())
	{
		UnbindAllBoundCharacters();
	}
	
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
	InteractionComponent->OnInteractReleased();
}

void AUnderwaterCharacter::M_StopCaptureState_Implementation()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			// SetIgnoreLookInput, SetIgnoreMoveInput은 내부적으로 Stack 같이 사용된다.
			// 도중에 Start Captrue, Stop Capture에서 순서를 맞추면 Character State에 관계없이 작동한다.
			PlayerController->SetIgnoreLookInput(false);
			PlayerController->SetIgnoreMoveInput(false);

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

void AUnderwaterCharacter::StartHealthRegen()
{
	if (GetCharacterState() != ECharacterState::Normal)
	{
		return;
	}
	
	const float HealthRegenAmount = StatComponent->GetMaxHealth() * HealthRegenRate;
	StatComponent->SetHealthRegenRate(HealthRegenAmount);
}

void AUnderwaterCharacter::StopHealthRegen()
{
	GetWorldTimerManager().ClearTimer(HealthRegenStartTimer);
	// 추후에 Buff나 Item으로 초당 회복을 실행한다고 변경되면 수정할 것
	StatComponent->SetHealthRegenRate(0.0f);
}

void AUnderwaterCharacter::StartCombat()
{
	if (bIsInCombat)
	{
		return;
	}

	bIsInCombat = true;

	if (IsLocallyControlled() && UnderwaterEffectComponent)
	{
		UnderwaterEffectComponent->StartCombatEffect();
	}

	StopHealthRegen();
	OnCombatStartDelegate.Broadcast();
}

void AUnderwaterCharacter::EndCombat()
{
	if (!bIsInCombat)
	{
		return;
	}

	bIsInCombat = false;

	if (IsLocallyControlled() && UnderwaterEffectComponent)
	{
		UnderwaterEffectComponent->StopCombatEffect();
	}

	if (GetCharacterState() == ECharacterState::Normal)
	{
		GetWorldTimerManager().SetTimer(HealthRegenStartTimer,
									this,
									&AUnderwaterCharacter::StartHealthRegen,
		                            HealthRegenDelay,
		                            false);
	}
	OnCombatEndDelegate.Broadcast();
}

void AUnderwaterCharacter::EndDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->SetHasBeenDead(true);
	}
	const float NewOxygenConsumeRate = OxygenComponent->GetConsumeRate() * 0.5f;
	OxygenComponent->SetConsumeRate(NewOxygenConsumeRate);
	
	if (AADPlayerController* PlayerController = Cast<AADPlayerController>(GetController()))
	{
		PlayerController->StartSpectate();
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter,Error, TEXT("PlayerController is not valid for %s"), *GetName());
	}
}

float AUnderwaterCharacter::GetSwimEffectiveSpeed() const
{
	const float BaseSpeed = StaminaComponent->IsSprinting()
		                        ? StatComponent->MoveSpeed * SprintMultiplier
		                        : StatComponent->MoveSpeed;

	// Effective Speed = BaseSpeed * (1 - OverloadSpeedFactor) * ZoneSpeedMultiplier
	//					* (1 - BindMultiplier)
	float Multiplier = 1.0f;
	if (IsOverloaded())
	{
		Multiplier = 1 - OverloadSpeedFactor;
	}
	Multiplier *= ZoneSpeedMultiplier;
	if (!BoundCharacters.IsEmpty())
	{
		Multiplier *= (1 - BindMultiplier * BoundCharacters.Num());
	}
	
	Multiplier = FMath::Max(0.0f, Multiplier);
	
	return FMath::Max(MinSpeed, BaseSpeed * Multiplier);
}

void AUnderwaterCharacter::AdjustSpeed()
{
	EffectiveSpeed = EnvironmentState == EEnvironmentState::Underwater
		? GetSwimEffectiveSpeed()
		: BaseGroundSpeed;

	// UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Adjust Speed : %s, EffectiveSpeed = %f"), *GetName(), EffectiveSpeed);
	
	if (EnvironmentState == EEnvironmentState::Underwater)
	{
		GetCharacterMovement()->MaxSwimSpeed = EffectiveSpeed;
	}
	else if (EnvironmentState == EEnvironmentState::Ground)
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

	FVector SpawnLocation = FirstPersonCameraComponent->GetComponentTransform().TransformPosition(RadarOffset);
	FRotator SpawnRotation = FirstPersonCameraComponent->GetComponentRotation() + RadarRotation;

	// @ToDO : Forward Actor에 맞추어서 Radar 회전
	RadarObject = GetWorld()->SpawnActor<ARadar>(RadarClass, SpawnLocation, SpawnRotation);
	RadarObject->AttachToComponent(FirstPersonCameraComponent, FAttachmentTransformRules::KeepWorldTransform);
	RadarObject->UpdateRadarSourceComponent(GetRootComponent(), GetRootComponent());
	RadarObject->SetActorHiddenInGame(true);
}

void AUnderwaterCharacter::SetMeshFirstPersonSetting(bool bIsFirstPerson)
{
	if (bIsFirstPerson)
	{
		GetMesh()->SetLightingChannels(false, true, false);
	}
	else
	{
		GetMesh()->SetLightingChannels(false, true, true);
	}
}

void AUnderwaterCharacter::OnLeavePawn()
{
	// 어차피 안 돌아올 것이므로 아무튼 숨김 처리한다.
	GetMesh1P()->SetHiddenInGame(true);

	if (LeftFlipperMesh1PComponent)
	{
		LeftFlipperMesh1PComponent->SetHiddenInGame(true);
	}
	if (RightFlipperMesh1PComponent)
	{
		RightFlipperMesh1PComponent->SetHiddenInGame(true);
	}

	SetMeshFirstPersonSetting(false);
}

void AUnderwaterCharacter::RequestToggleRadar()
{
	if (HasAuthority())
	{
		bIsRadarOn = !bIsRadarOn;
		OnRep_bIsRadarOn();
	}
	else
	{
		S_ToggleRadar();
	}
}

void AUnderwaterCharacter::UpdateBlurEffect()
{
	// 레이더가 켜져 있으면 Blur 효과를 꺼야 한다.
	// 레이더가 꺼져 있으면 수중일 경우 Blur 효과를 켜고 지상일 경우 Blur 효과를 끈다.
	const bool bShouldEnableBlur = EnvironmentState == EEnvironmentState::Underwater && !bIsRadarOn;
	SetBlurEffect(bShouldEnableBlur);
}

void AUnderwaterCharacter::SetBlurEffect(const bool bEnable)
{
	// Post Effect 효과는 Local Player에서만 적용한다.
	if (!IsLocallyControlled())
	{
		return;
	}

	FirstPersonCameraComponent->PostProcessSettings.bOverride_MotionBlurAmount = !bEnable;
	FirstPersonCameraComponent->PostProcessSettings.MotionBlurAmount = 0.0f;
}

void AUnderwaterCharacter::SetRadarVisibility(bool bRadarVisible)
{
	if (!IsValid(RadarObject) || !IsLocallyControlled())
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
	UpdateBlurEffect();
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

	const EEnvironmentState NewEnvironmentState = NewVolume->bWaterVolume ? EEnvironmentState::Underwater : EEnvironmentState::Ground;
	LOG_NETWORK(LogAbyssDiverCharacter, Display, TEXT("Physics Volume Changed : %s / State : %s"),
		*NewVolume->GetName(),
		NewVolume->bWaterVolume ? TEXT("Underwater") : TEXT("Ground"));
	SetEnvironmentState(NewEnvironmentState);
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

		if (EmoteAction1)
		{
			EnhancedInput->BindAction(
				EmoteAction1,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::PerformEmote1
			);
		}

		if (EmoteAction2)
		{
			EnhancedInput->BindAction(
				EmoteAction2,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::PerformEmote2
			);
		}

		if (EmoteAction3)
		{
			EnhancedInput->BindAction(
				EmoteAction3,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::PerformEmote3
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
	if (!HasAuthority())
	{
		return 0.0f;
	}
	if (bIsInvincible)
	{
		return 0.0f;
	}

	if (CharacterState == ECharacterState::Groggy)
	{
		const float NewRemainGroggyTime = GetRemainGroggyTime() - GroggyHitPenalty;
		if (NewRemainGroggyTime <= 0.0f)
		{
			SetCharacterState(ECharacterState::Death);
			return 0.0f;
		}
		else
		{
			GetWorldTimerManager().SetTimer(GroggyTimer,
			                                FTimerDelegate::CreateUObject(
				                                this, &AUnderwaterCharacter::SetCharacterState, ECharacterState::Death),
			                                NewRemainGroggyTime,
			                                false);
			return 0.0f;
		}
	}
	else if (CharacterState == ECharacterState::Death)
	{
		return 0.0f;
	}

	// 정해져야 할 것
	// 1. EmitBloodNoise를 Shield만 소모됬을 때 호출할 것인지

	const FShieldAbsorbResult ShieldAbsorbResult = ShieldComponent->AbsorbDamage(DamageAmount);
	// UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Take Damage : %f, Remaining Damage : %f"), DamageAmount, ShieldAbsorbResult.RemainingDamage);

	// Shield Over Damage를 Stat Component에서 계산
	const float ActualDamage = Super::TakeDamage(ShieldAbsorbResult.RemainingDamage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f)
	{
		EmitBloodNoise();
	}

	// Normal State
	// 1. Shield Logic
	// 2. Health Logic(StatComponent)
	// 2.1 Health가 0이 되면 Groggy로 전이(OnHealthChanged 함수에서 처리), 이 밑으로는 State 전이가 발생되어 있다.
	// 3. OnDamageTakenDelegate

	// OnDamageTaken 함수를 이용할 경우 발생한 순간 Groggy 상태로 전이될 수 있다.
	// Normal State에만 적용되어야 하는 로직이라면 캐릭터의 현재 상태를 검사해야 한다.
	// Current Health를 이용할 수 있지만 Capture State를 Polishing 할 경우 Groggy 상태가 Pending 될 수 있다.
	// 캐릭터의 상태를 검사하는 것이 더 안전하다.
	OnDamageTakenDelegate.Broadcast(ActualDamage, StatComponent->GetCurrentHealth());

	StopHealthRegen();
	if (CharacterState == ECharacterState::Normal
		&& !bIsInCombat)
	{
		GetWorldTimerManager().SetTimer(
			HealthRegenStartTimer,
			this,
			&AUnderwaterCharacter::StartHealthRegen,
			HealthRegenDelay,
			false
		);
	}

	return ActualDamage;
}

void AUnderwaterCharacter::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Interact : %s"), *InstigatorActor->GetName());
	if (!HasAuthority())
	{
		return;
	}

	if (CharacterState == ECharacterState::Death)
	{
		if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(InstigatorActor))
		{
			RequestBind(UnderwaterCharacter);
		}
	}
}

void AUnderwaterCharacter::InteractHold_Implementation(AActor* InstigatorActor)
{
	LOGN(TEXT("Interact Hold : %s"), *GetName());
	if (!HasAuthority())
	{
		return;
	}
	
	if (CharacterState == ECharacterState::Groggy)
	{
		RequestRevive();
	}
}

void AUnderwaterCharacter::OnHoldStart_Implementation(APawn* InstigatorPawn)
{
}

void AUnderwaterCharacter::OnHoldStop_Implementation(APawn* InstigatorPawn)
{
	// Hold 모드가 아닐 경우에도 호출이 되므로 주의
}

bool AUnderwaterCharacter::CanHighlight_Implementation() const
{
	return InteractableComponent->CanInteractable();
}

float AUnderwaterCharacter::GetHoldDuration_Implementation(AActor* InstigatorActor) const
{
	return RescueRequireTime;
}

UADInteractableComponent* AUnderwaterCharacter::GetInteractableComponent() const
{
	return InteractableComponent;
}

bool AUnderwaterCharacter::IsHoldMode() const
{
	return bIsInteractionHoldMode;
}

FString AUnderwaterCharacter::GetInteractionDescription() const
{
	return InteractionDescription;
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
		const float RecoveryOxygen = OxygenComponent->GetOxygenLevel() * RecoveryOxygenPenaltyRate;
		OxygenComponent->InitOxygenSystem(RecoveryOxygen, OxygenComponent->GetMaxOxygenLevel());
	}
	else
	{
		S_Revive();
	}
}

void AUnderwaterCharacter::Die()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCharacterState(ECharacterState::Death);
}

void AUnderwaterCharacter::Respawn()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Respawn Character : %s"),
		GetPlayerState<AADPlayerState>() ? *GetPlayerState<AADPlayerState>()->GetPlayerNickname() : TEXT("Unknown"));

	if (AADPlayerController* PlayerController = Cast<AADPlayerController>(GetController()))
	{
		UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Respawn PlayerController : %s"), *PlayerController->GetName());
		PlayerController->C_PlaySound(ResurrectSFX);
	}
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		const float MaxOxygenLevel = OxygenComponent->GetMaxOxygenLevel();
		float RespawnOxygenLevel = PS->GetLastOxygenRemain() * 0.3f;
		RespawnOxygenLevel = FMath::Max(MaxOxygenLevel * 0.2f, RespawnOxygenLevel);
		OxygenComponent->InitOxygenSystem(RespawnOxygenLevel, MaxOxygenLevel);
	}
}

void AUnderwaterCharacter::Move(const FInputActionValue& InputActionValue)
{
	// To-Do
	// Can Move 확인
	
	// 캐릭터의 XYZ 축을 기준으로 입력을 받는다.
	const FVector MoveInput = InputActionValue.Get<FVector>();

	if (EnvironmentState == EEnvironmentState::Ground)
	{
		MoveGround(MoveInput);
	}
	else if (EnvironmentState == EEnvironmentState::Underwater)
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
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (bPlayingEmote && AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		RequestStopPlayingEmote(PlayEmoteIndex);
	}
	
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
	if (EnvironmentState == EEnvironmentState::Underwater)	
	{
		// 수중에서는 점프가 불가능하다.
		return;
	}

	if (bPlayingEmote && GetMesh()->GetAnimInstance() && GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		RequestStopPlayingEmote(PlayEmoteIndex);
	}
	
	Jump();
}

void AUnderwaterCharacter::JumpInputStop(const FInputActionValue& InputActionValue)
{
	if (EnvironmentState == EEnvironmentState::Underwater)
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

void AUnderwaterCharacter::PerformEmote1(const FInputActionValue& InputActionValue)
{
	if (EnvironmentState == EEnvironmentState::Ground && GetCharacterMovement()->IsMovingOnGround()
		&& !bPlayingEmote)
	{
		RequestPlayEmote(0);
	}
}

void AUnderwaterCharacter::PerformEmote2(const FInputActionValue& InputActionValue)
{
	if (EnvironmentState == EEnvironmentState::Ground && GetCharacterMovement()->IsMovingOnGround()
		&& !bPlayingEmote)
	{
		RequestPlayEmote(1);
	}
}

void AUnderwaterCharacter::PerformEmote3(const FInputActionValue& InputActionValue)
{
	if (EnvironmentState == EEnvironmentState::Ground && GetCharacterMovement()->IsMovingOnGround()
		&& !bPlayingEmote)
	{
		RequestPlayEmote(2);
	}
}

void AUnderwaterCharacter::SetDebugCameraMode(bool bDebugCameraEnable)
{
	bUseDebugCamera = bDebugCameraEnable;
	if (bUseDebugCamera)
	{
		FirstPersonCameraComponent->SetActive(false);
		ThirdPersonCameraComponent->SetActive(true);
		GetMesh()->SetOwnerNoSee(false);
		GetMesh1P()->SetOwnerNoSee(true);
	}
	else
	{
		FirstPersonCameraComponent->SetActive(true);
		ThirdPersonCameraComponent->SetActive(false);
		GetMesh()->SetOwnerNoSee(true);
		GetMesh1P()->SetOwnerNoSee(false);
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

void AUnderwaterCharacter::RequestPlayEmote(int8 EmoteIndex)
{
	// Server, Client 모두 Emote Index를 검증한다.
	if (EmoteIndex >= EmoteAnimationMontages.Num())
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Emote Index %d is out of range"), EmoteIndex);
		return;
	}
	
	if (HasAuthority())
	{
		if (CanPlayEmote())
		{
			M_BroadcastPlayEmote(EmoteIndex);
		}
	}
	else
	{
		S_PlayEmote(EmoteIndex);
	}
}

void AUnderwaterCharacter::S_PlayEmote_Implementation(uint8 EmoteIndex)
{
	RequestPlayEmote(EmoteIndex);
}

void AUnderwaterCharacter::M_BroadcastPlayEmote_Implementation(int8 EmoteIndex)
{
	UAnimMontage* EmoteMontage = EmoteAnimationMontages[EmoteIndex];
	if (EmoteMontage == nullptr)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Emote Montage is not valid for index %d"), EmoteIndex);
		return;
	}

	bPlayingEmote = true;
	PlayEmoteIndex = EmoteIndex;
	
	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Play Emote Montage : %s"), *EmoteMontage->GetName());
		AnimInstance->Montage_Play(EmoteMontage, 1.0f);
			
		FOnMontageEnded MontageEndDelegate;
		MontageEndDelegate.BindUObject(this, &AUnderwaterCharacter::OnEmoteEnd);
		AnimInstance->Montage_SetEndDelegate(MontageEndDelegate, EmoteMontage);
	}

	// Control Rotation을 이용해서 Rotation을 갱신하기 때문에 모든 노드에서 동기화가 되어야 한다.
	bUseControllerRotationYaw = false;

	if (IsLocallyControlled())
	{
		StartEmoteCameraTransition();
	}
	StartEmoteCameraTransition();
}

bool AUnderwaterCharacter::CanPlayEmote() const
{
	const UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	const bool bIsMontagePlaying = AnimInstance && AnimInstance->IsAnyMontagePlaying();
	return !bPlayingEmote && !bIsMontagePlaying; 
}

UAnimMontage* AUnderwaterCharacter::GetEmoteMontage(int8 EmoteIndex) const
{
	return EmoteAnimationMontages.IsValidIndex(EmoteIndex)
		? EmoteAnimationMontages[EmoteIndex]
		: nullptr;
}

void AUnderwaterCharacter::RequestStopPlayingEmote(int8 EmoteIndex)
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Request Stop Playing Emote / Emote Index : %d, bPlayingEmote : %s"),
		EmoteIndex, bPlayingEmote ? TEXT("True") : TEXT("False"));
	// Server, Client 모두 Emote Index를 검증한다.
	if (!EmoteAnimationMontages.IsValidIndex(EmoteIndex))
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Emote Index %d is out of range"), EmoteIndex);
		return;
	}
	
	if (HasAuthority())
	{
		M_BroadcastStopPlyingEmote(EmoteIndex);
	}
	else
	{
		S_StopPlayingEmote(EmoteIndex);
	}
}

void AUnderwaterCharacter::M_BroadcastStopPlyingEmote_Implementation(int8 EmoteIndex)
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Broadcast Stop Playing Emote / Emote Index : %d, bPlayingEmote : %s"),
		EmoteIndex, bPlayingEmote ? TEXT("True") : TEXT("False"));
	// Broadcast 도중에 Emote가 이미 종료되었다면 종료
	if (bPlayingEmote == false)
	{
		return;	
	}
	
	UAnimMontage* EmoteMontage = GetEmoteMontage(EmoteIndex);
	if (EmoteMontage == nullptr)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("Emote Montage is not valid for index %d"), EmoteIndex);
		return;
	}

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Force Stop Playing Emote : %s"), *EmoteMontage->GetName());
	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (AnimInstance->Montage_IsPlaying(EmoteMontage))
		{
			AnimInstance->Montage_Stop(0.1f, EmoteMontage);
		}
	}

	// Control Rotation은 Emote Camera Transition을 기다리지 않고 바로 반영한다.
	bUseControllerRotationYaw = true;
}

void AUnderwaterCharacter::S_StopPlayingEmote_Implementation(int8 EmoteIndex)
{
	RequestStopPlayingEmote(EmoteIndex);
}

void AUnderwaterCharacter::OnEmoteEnd(UAnimMontage* AnimMontage, bool bInterupted)
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("OnEmoteEnd : %s, bArg : %d"), *AnimMontage->GetName(), bInterupted);

	CameraTransitionDirection = -1.0f;
}

void AUnderwaterCharacter::StartEmoteCameraTransition()
{
	CameraTransitionDirection = 1.0f;
	CameraTransitionTimeElapsed = 0.0f;
	SetCameraFirstPerson(false);
	// FirstPersonCameraArm->bInheritRoll = true;
	GetWorldTimerManager().SetTimer(
		EmoteCameraTransitionTimer,
		this,
		&AUnderwaterCharacter::UpdateCameraTransition,
		CameraTransitionUpdateInterval,
		true
	);
}

void AUnderwaterCharacter::SetCameraFirstPerson(bool bFirstPersonCamera)
{
	GetMesh()->SetOwnerNoSee(bFirstPersonCamera);
	GetMesh1P()->SetOwnerNoSee(!bFirstPersonCamera);
}

void AUnderwaterCharacter::UpdateCameraTransition()
{
	if (CameraTransitionDuration <= 0.0f || CameraTransitionUpdateInterval <= 0.0f)
	{
		UE_LOG(LogAbyssDiverCharacter, Error, TEXT("Camera Transition Duration or Update Interval is not set properly. Duration: %f, Update Interval: %f"),
			CameraTransitionDuration, CameraTransitionUpdateInterval);
		return;
	}
	
	CameraTransitionTimeElapsed += CameraTransitionUpdateInterval * CameraTransitionDirection;
	CameraTransitionTimeElapsed = FMath::Clamp(CameraTransitionTimeElapsed, 0.0f, CameraTransitionDuration);
	const float Alpha = CameraTransitionTimeElapsed / CameraTransitionDuration;

	// First Person Camera Transition End
	if (Alpha <= 0.0f && CameraTransitionDirection < 0.0f)
	{
		GetWorldTimerManager().ClearTimer(EmoteCameraTransitionTimer);
		bPlayingEmote = false;
		SetCameraFirstPerson(true);
		// FirstPersonCameraArm->bInheritRoll = false;
		return;
	}
	
	const float NewSpringArmLength = UKismetMathLibrary::Ease(
		0.0f,
		EmoteCameraTransitionLength,
		Alpha,
		EmoteCameraTransitionEasingType
	);
	FirstPersonCameraArm->TargetArmLength = NewSpringArmLength;
}

void AUnderwaterCharacter::OnRep_BindCharacter()
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("OnRep_BindCharacter : %s"), *GetName());
	if (BindCharacter)
	{
		ConnectRope(BindCharacter);
	}
	else
	{
		DisconnectRope();
	}

	UpdateBindInteractable();
}

void AUnderwaterCharacter::OnRep_BoundCharacters()
{
	AdjustSpeed();
}

void AUnderwaterCharacter::BindToCharacter(AUnderwaterCharacter* BoundCharacter)
{
	if (BoundCharacter == nullptr)
	{
		return;
	}

	BoundCharacters.Add(BoundCharacter);
	AdjustSpeed();

	LOGVN(Display, TEXT("Binder : %s, Bound : %s"), *GetName(), *BoundCharacter->GetName());
}

void AUnderwaterCharacter::UnbindToCharacter(AUnderwaterCharacter* BoundCharacter)
{
	BoundCharacters.Remove(BoundCharacter);
	AdjustSpeed();
}

void AUnderwaterCharacter::ConnectRope(AUnderwaterCharacter* BinderCharacter)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CableBindingActor = GetWorld()->SpawnActor<ACableBindingActor>(	
		CableBindingActorClass, 
		BinderCharacter->GetActorLocation(), 
		FRotator::ZeroRotator, 
		SpawnParams
	);

	if (CableBindingActor)
	{
		CableBindingActor->ConnectActors(BinderCharacter, this);
	}
}

void AUnderwaterCharacter::DisconnectRope()
{
	if (CableBindingActor)
	{
		UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Disconnect Rope : %s / Node : %s"), *GetName(), HasAuthority() ? TEXT("Host") : TEXT("Client"));
		CableBindingActor->DisconnectActors();

		CableBindingActor->Destroy();
		CableBindingActor = nullptr;
	}
}

void AUnderwaterCharacter::UpdateBindInteractable()
{
	// Bind 상태는 Death일 경우에만 호출
	if (CharacterState != ECharacterState::Death)
	{
		return;
	}
	
	// Binder Character가 nullptr일 경우 연결된 Bind 상태가 아니므로 활성화
	if (BindCharacter == nullptr)
	{
		InteractableComponent->SetInteractable(true);
		InteractionDescription = DeathGrabDescription;
		InteractionComponent->PerformFocusCheck();
	}
	// Binder Character가 Local Controller일 경우 활성화해서 Unbind를 가능하도록 수정
	else if (BindCharacter->IsLocallyControlled())
	{
		InteractableComponent->SetInteractable(true);
		InteractionDescription = DeathGrabReleaseDescription;
		InteractionComponent->PerformFocusCheck();
	}
	else
	{
		InteractableComponent->SetInteractable(false);
		InteractionDescription = DeathGrabDescription;
		InteractionComponent->PerformFocusCheck();
	}
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

void AUnderwaterCharacter::SetZoneSpeedMultiplier(float NewMultiplier)
{
	ZoneSpeedMultiplier = NewMultiplier;
	AdjustSpeed();
}

bool AUnderwaterCharacter::IsWeaponEquipped() const
{
	return EquipUseComponent ? EquipUseComponent->bIsWeapon : false;
}

UUserWidget* AUnderwaterCharacter::GetShieldHitWidget() const
{
	return CombatEffectComponent ? CombatEffectComponent->GetShieldHitWidget() : nullptr;
}
