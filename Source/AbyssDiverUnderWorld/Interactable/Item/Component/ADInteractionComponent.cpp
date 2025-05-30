#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Components/SphereComponent.h"
#include "ADInteractableComponent.h"
#include "DrawDebugHelpers.h"
#include "AbyssDiverUnderWorld.h"
#include "Interface/IADInteractable.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UADInteractionComponent::UADInteractionComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bHoldTriggered = false;
	bIsFocusing = false;
	bIsInteractingStart = false;
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
	if (!bIsFocusing && bIsInteractingStart)
	{
		if (IsLocallyControlled())
		{
			OnInteractReleased();
		}
	}
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
	if (IIADInteractable* IADInteractable = Cast<IIADInteractable>(TargetActor))
	{
		if (UADInteractableComponent* ADIC = IADInteractable->GetInteractableComponent())
		{
			APawn* Pawn = Cast<APawn>(GetOwner());
			if (Pawn && Pawn->HasAuthority())
			{
				ADIC->Interact(Pawn);
				LOGIC(Log, TEXT("Pawn HasAuthority"));
			}
		}
	}

}

void UADInteractionComponent::S_RequestInteractHold_Implementation(AActor* TargetActor)
{
	bIsInteractingStart = true;
	if (IIADInteractable* IADInteractable = Cast<IIADInteractable>(TargetActor))
	{
		FocusedInteractable = IADInteractable->GetInteractableComponent();
		HoldThreshold = IIADInteractable::Execute_GetHoldDuration(TargetActor);
		if (!FocusedInteractable)
		{
			return;
		}

		if (!TargetActor) return;
		HandleInteractPressed(TargetActor);
	}
}

void UADInteractionComponent::S_RequestInteractRelease_Implementation()
{
	HandleInteractReleased();
}


void UADInteractionComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IIADInteractable* IADInteractable = Cast<IIADInteractable>(OtherActor))
	{
		if (UADInteractableComponent* ADIC = IADInteractable->GetInteractableComponent())
		{
			NearbyInteractables.Add(ADIC);
			SetComponentTickEnabled(true);
			//		LOG(TEXT("Overlapped!"));
		}
	}
}

void UADInteractionComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IIADInteractable* IADInteractable = Cast<IIADInteractable>(OtherActor))
	{
		if (UADInteractableComponent* ADIC = IADInteractable->GetInteractableComponent())
		{
			NearbyInteractables.Remove(ADIC);
			if (NearbyInteractables.Num() == 0)
			{
				SetComponentTickEnabled(false);

				if (FocusedInteractable)
				{
					// 외곽선 제거
					FocusedInteractable->SetHighLight(false);
					OnFocusEnd.Broadcast();
					FocusedInteractable = nullptr;
					//				LOG(TEXT("Remove Border"));
				}
				//			LOG(TEXT("No Interactable!!"));

			}
		}
		//	LOG(TEXT("Overlapp End!!"));
	}
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
			LOGIC(Log, TEXT("Server"));
			// 호스트 모드라면 바로 호출
			FocusedInteractable->Interact(Pawn);
		}
		else
		{
			LOGIC(Log, TEXT("Client"));
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
		bIsFocusing = true;
		UpdateFocus(HitInteractable);
	}
	else
	{
		bIsFocusing = false;
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
		if (IIADInteractable * IADInteractable = Cast<IIADInteractable>(Hit.GetActor()))
		{
			return IADInteractable->GetInteractableComponent();
		}
	}
	return nullptr;
}

void UADInteractionComponent::UpdateFocus(UADInteractableComponent* NewFocus)
{
//	if (!IsLocallyControlled()) return;
	if (NewFocus == FocusedInteractable) 
	{
		if (!ShouldHighlight(NewFocus))
		{
			ClearFocus();
		}
		return;
	}

	if (FocusedInteractable)
	{
		FocusedInteractable->SetHighLight(false);
		OnFocusEnd.Broadcast();
	}
		

	if (ShouldHighlight(NewFocus))
	{
		FocusedInteractable = NewFocus;
		FocusedInteractable->SetHighLight(true);
		if (IIADInteractable* FocusedActor = Cast<IIADInteractable>(FocusedInteractable->GetOwner()))
		{
			OnFocus.Broadcast(FocusedInteractable->GetOwner(), FocusedActor->GetInteractionDescription());
		}
		
	}
	else
	{
		FocusedInteractable = nullptr;
	}
}

void UADInteractionComponent::ClearFocus()
{
//	if (!IsLocallyControlled()) return;
	if (FocusedInteractable)
	{
		FocusedInteractable->SetHighLight(false);
		OnFocusEnd.Broadcast();
		FocusedInteractable = nullptr;
	}
}

void UADInteractionComponent::OnInteractPressed()
{
	if (!FocusedInteractable) return;
	bIsInteractingStart = true;
	if (AActor* Owner = FocusedInteractable->GetOwner())
	{
		
		if (IIADInteractable* InteractableOwner = Cast<IIADInteractable>(Owner))
		{

			const bool bIsHoldMode = InteractableOwner->IsHoldMode();
			if (!bIsHoldMode)
			{
				TryInteract();   // 즉시 수행
				return;
			}
			HoldThreshold = IIADInteractable::Execute_GetHoldDuration(Owner);
			OnHoldStart.Broadcast(Owner, HoldThreshold);
			// Hold Mode일 경우
			if (GetOwner()->HasAuthority())
			{
				HandleInteractPressed(Owner);
			}
			else
			{
				// 클라이언트라면 RPC 호출
				S_RequestInteractHold(Owner);
			}
		}
		
	}
}

void UADInteractionComponent::OnInteractReleased()
{
	if (!bIsInteractingStart)
		return;

	bIsInteractingStart = false;

	// Hold 오브젝트지만 시간 100% 못 채우고 놓은 경우: 취소
	if (!bHoldTriggered)
	{
		if (GetOwner()->HasAuthority())
		{
			HandleInteractReleased();
		}
		else
		{
			S_RequestInteractRelease();
		}
	}
	OnHoldCancel.Broadcast();
}

void UADInteractionComponent::OnHoldComplete()
{
	bHoldTriggered = true;
	AActor* Instigator = HoldInstigator.Get();
	if (!Instigator || !FocusedInteractable) return;

	LOGIC(Log, TEXT("End Hold!"));
	IIADInteractable::Execute_InteractHold(FocusedInteractable->GetOwner(), Instigator);

	bIsInteractingStart = false;
}

void UADInteractionComponent::HandleInteractPressed(AActor* TargetActor)
{
	CachedHoldInteractable = FocusedInteractable;

	if (TargetActor->GetClass()->ImplementsInterface(UIADInteractable::StaticClass()))
	{
		IIADInteractable::Execute_OnHoldStart(TargetActor, Cast<APawn>(GetOwner()));

		// == Hold 모드 ==
		bHoldTriggered = false;
		HoldInstigator = Cast<APawn>(GetOwner());

		LOGIC(Log, TEXT("Start Hold!"));
		GetWorld()->GetTimerManager().SetTimer(
			HoldTimerHandle, this,
			&UADInteractionComponent::OnHoldComplete,
			HoldThreshold, false);
	}
}

void UADInteractionComponent::HandleInteractReleased()
{
	bIsInteractingStart = false;
	GetWorld()->GetTimerManager().ClearTimer(HoldTimerHandle);
	LOGIC(Log, TEXT("Fail Hold!"));

	if (CachedHoldInteractable.IsValid() && HoldInstigator.Get())
	{
		APawn* InstigatorPawn = Cast<APawn>(HoldInstigator.Get());
		if (InstigatorPawn)
		{
			IIADInteractable::Execute_OnHoldStop(CachedHoldInteractable->GetOwner(), InstigatorPawn);
		}
		
	}
		
	HoldInstigator = nullptr;
	CachedHoldInteractable = nullptr;
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

void UADInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UADInteractionComponent, bIsInteractingStart);
	DOREPLIFETIME(UADInteractionComponent, bHoldTriggered);
	DOREPLIFETIME(UADInteractionComponent, bIsFocusing);
}

