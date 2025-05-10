#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Components/SphereComponent.h"
#include "ADInteractableComponent.h"
#include "DrawDebugHelpers.h"
#include "AbyssDiverUnderWorld.h"

// Sets default values for this component's properties
UADInteractionComponent::UADInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}


// Called when the game starts
void UADInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 런타임 전용으로 생성
	RangeSphere = NewObject<USphereComponent>(GetOwner(), TEXT("InteractionRange"));
	RangeSphere->AttachToComponent(
		GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform
	);

	RangeSphere->InitSphereRadius(400.f);
	RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
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
			// 호스트 모드라면 바로 호출
			FocusedInteractable->Interact(Pawn);
		}
		else
		{
			// TODO : 클라이언트 → 서버 요청 (서버 RPC 필요)
		}
	}
}

void UADInteractionComponent::PerformFocusCheck()
{
	if (NearbyInteractables.Num() == 0) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	FVector CamLocation;
	FRotator CamRotation;
	PC->GetPlayerViewPoint(CamLocation, CamRotation);

	const float TraceDist = RangeSphere->GetScaledSphereRadius();
	FVector End = CamLocation + CamRotation.Vector() * TraceDist;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());


	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, CamLocation, End, ECC_Visibility, Params
	);


	/*FColor LineColor = bHit ? FColor::Blue : FColor::Red;


	DrawDebugLine(
		GetWorld(),
		CamLocation,
		End,
		LineColor,
		false,  
		2.0f,   
		0,
		1.5f   
	);*/

	if (bHit)
	{
//		LOG(TEXT("Hit!!"));
		if (UADInteractableComponent* ADIC = Hit.GetActor()->FindComponentByClass<UADInteractableComponent>())
		{
//			LOG(TEXT("Is ADInteractable"));
			if (NearbyInteractables.Contains(ADIC))
			{
//				LOG(TEXT("Contain ADIC"));
				if (ADIC != FocusedInteractable)
				{
//					LOG(TEXT("ADIC is not FocusInteractable"));
					if (FocusedInteractable)
						FocusedInteractable->SetHighLight(false);

					FocusedInteractable = ADIC;
					FocusedInteractable->SetHighLight(true);
				}
				return;
			}
		}
	}

	if (FocusedInteractable)
	{
		FocusedInteractable->SetHighLight(false);
		FocusedInteractable = nullptr;
	}
}


void UADInteractionComponent::OnInteractPressed()
{
	PerformFocusCheck();
	if (FocusedInteractable)
	{
		FocusedInteractable->Interact(GetOwner());
	}
}

