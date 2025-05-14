#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Components/SphereComponent.h"
#include "ADInteractableComponent.h"
#include "DrawDebugHelpers.h"
#include "AbyssDiverUnderWorld.h"
#include "Interface/IADInteractable.h"

// Sets default values for this component's properties
UADInteractionComponent::UADInteractionComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bHoldTriggered = false;
}


// Called when the game starts
void UADInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			/** 로컬 플레이어가 아니면 하이라이트 계산 자체를 끈다 */
			if (!PC->IsLocalController())
			{
				PrimaryComponentTick.SetTickFunctionEnable(false);
				return;                     // 밑의 스피어도 만들 필요 없음
			}
		}
	}

	// 런타임 전용으로 생성
	RangeSphere = NewObject<USphereComponent>(GetOwner(), TEXT("InteractionRange"));
	RangeSphere->AttachToComponent(
		GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform
	);

	RangeSphere->InitSphereRadius(400.f);
	RangeSphere->SetCollisionProfileName(TEXT("Interaction"));
	RangeSphere->SetGenerateOverlapEvents(true);


	RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UADInteractionComponent::HandleBeginOverlap);
	RangeSphere->OnComponentEndOverlap.AddDynamic(this, &UADInteractionComponent::HandleEndOverlap);
	RangeSphere->RegisterComponent();
}


void UADInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	PerformFocusCheck();

	DrawDebugSphere(
		GetWorld(),
		GetOwner()->GetActorLocation(),
		RangeSphere->GetScaledSphereRadius(),
		16, FColor::Blue, false, -1.f, 0, 2.f
	);
}

void UADInteractionComponent::S_RequestInteract_Implementation(AActor* TargetActor)
{
	if (!TargetActor) return;

	if (UADInteractableComponent* ADIC = TargetActor->FindComponentByClass<UADInteractableComponent>())
	{
		APawn* Pawn = Cast<APawn>(GetOwner());
		if (Pawn && Pawn->HasAuthority())
		{
			ADIC->Interact(Pawn);
			LOG(TEXT("Pawn HasAuthority"));
		}
			
	}
}

void UADInteractionComponent::S_RequestInteractHold_Implementation(AActor* TargetActor)
{
	if (!TargetActor) return;

	IIADInteractable::Execute_InteractHold(TargetActor, GetOwner());
}


void UADInteractionComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UADInteractableComponent* ADIC = OtherActor->FindComponentByClass<UADInteractableComponent>())
	{
		NearbyInteractables.Add(ADIC);
		SetComponentTickEnabled(true);
//		LOG(TEXT("Overlapped!"));
	}
}

void UADInteractionComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UADInteractableComponent* ADIC = OtherActor->FindComponentByClass<UADInteractableComponent>())
	{
		NearbyInteractables.Remove(ADIC);
		if (NearbyInteractables.Num() == 0)
		{
			SetComponentTickEnabled(false);

			if (FocusedInteractable)
			{
				// 외곽선 제거
				FocusedInteractable->SetHighLight(false);
				FocusedInteractable = nullptr;
//				LOG(TEXT("Remove Border"));
			}
//			LOG(TEXT("No Interactable!!"));

		}
	}
//	LOG(TEXT("Overlapp End!!"));
}

void UADInteractionComponent::TryInteract()
{
	PerformFocusCheck();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	// 포커싱 판정 로직...
	if (FocusedInteractable)
	{
		if (Pawn->HasAuthority())
		{
			LOG(TEXT("Server"));
			// 호스트 모드라면 바로 호출
			FocusedInteractable->Interact(Pawn);
		}
		else
		{
			LOG(TEXT("Client"));
			S_RequestInteract(FocusedInteractable->GetOwner());
		}
	}
}

void UADInteractionComponent::PerformFocusCheck()
{
	if (!IsLocallyControlled()) return;
	if (NearbyInteractables.Num() == 0) return;

	FVector CamLocation;
	FVector TraceEnd;
	if (!ComputeViewTrace(CamLocation, TraceEnd))
	{
		ClearFocus();
		return;
	}

	UADInteractableComponent* HitInteractable = PerformLineTrace(CamLocation, TraceEnd);
	if (HitInteractable && NearbyInteractables.Contains(HitInteractable))
	{
		UpdateFocus(HitInteractable);
	}
	else
	{
		ClearFocus();
	}
}

bool UADInteractionComponent::ComputeViewTrace(FVector& OutStart, FVector& OutEnd) const
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return false;
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());	
	if (!PC) return false;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	OutStart = CamLoc;
	OutEnd = CamLoc + CamRot.Vector() * RangeSphere->GetScaledSphereRadius();
	return true;
}

UADInteractableComponent* UADInteractionComponent::PerformLineTrace(const FVector& Start, const FVector& End) const
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetActor()->FindComponentByClass<UADInteractableComponent>();
	}
	return nullptr;
}

void UADInteractionComponent::UpdateFocus(UADInteractableComponent* NewFocus)
{
	if (!IsLocallyControlled()) return;
	if (NewFocus == FocusedInteractable) return;

	if (FocusedInteractable)
		FocusedInteractable->SetHighLight(false);

	if (ShouldHighlight(NewFocus))
	{
		FocusedInteractable = NewFocus;
		FocusedInteractable->SetHighLight(true);
	}
	else
	{
		FocusedInteractable = nullptr;
	}
}

void UADInteractionComponent::ClearFocus()
{
	if (!IsLocallyControlled()) return;
	if (FocusedInteractable)
	{
		FocusedInteractable->SetHighLight(false);
		FocusedInteractable = nullptr;
	}
}

void UADInteractionComponent::OnInteractPressed()
{
	PerformFocusCheck();
	if (!FocusedInteractable) return;

	bHoldTriggered = false;
	HoldInstigator = Cast<APawn>(GetOwner());

	GetWorld()->GetTimerManager().SetTimer(
		HoldTimerHandle,
		this,
		&UADInteractionComponent::OnHoldComplete,
		HoldThreshold,
		false
	);
}

void UADInteractionComponent::OnInteractReleased()
{
	if (!bHoldTriggered && FocusedInteractable)
	{
		GetWorld()->GetTimerManager().ClearTimer(HoldTimerHandle);

		if (GetOwner()->HasAuthority())
		{
			FocusedInteractable->Interact(Cast<APawn>(GetOwner()));
		}
		else
		{
			// 클라이언트 → 서버 RPC
			S_RequestInteract(FocusedInteractable->GetOwner());
		}
	}
	HoldInstigator = nullptr;
}


void UADInteractionComponent::OnHoldComplete()
{
	bHoldTriggered = true;
	AActor* Instigator = HoldInstigator.Get();
	if (!Instigator || !FocusedInteractable) return;

	if (GetOwner()->HasAuthority())
	{
		IIADInteractable::Execute_InteractHold(FocusedInteractable->GetOwner(), Instigator);
	}
	else
	{
		S_RequestInteractHold(FocusedInteractable->GetOwner());
	}
}

bool UADInteractionComponent::ShouldHighlight(const UADInteractableComponent* ADIC) const
{
	if (!ADIC) return false;
	AActor* TargetActor = ADIC->GetOwner();
	return IIADInteractable::Execute_CanHighlight(TargetActor);
}

bool UADInteractionComponent::IsLocallyControlled() const
{
	if (const APawn* P = Cast<APawn>(GetOwner()))
		if (const AController* C = P->GetController())
			return C->IsLocalController();
	return false;
}

