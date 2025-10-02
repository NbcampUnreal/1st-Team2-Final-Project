// Fill out your copyright notice in the Description page of Project Settings.


#include "NameWidgetComponent.h"

#include "Kismet/GameplayStatics.h"
#include "UI/ADNameWidget.h"

UNameWidgetComponent::UNameWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetUsingAbsoluteRotation(true);
	SetWidgetSpace(EWidgetSpace::World);
	DrawSize = FIntPoint(200, 100);
	
	DisplayDistance = 1000.0f;
	MaxDisplayDistance = 1500.0f;
	bIsEnabled = false;
	bIsVisible = false;
	BillboardRotationMode = EBillboardRotationMode::ReverseCameraForward;

	SetCastShadow(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UNameWidgetComponent::BeginPlay()
{
	// WidgetComponent에서 InitWidget을 호출하고 있으므로 명시적으로 생성할 필요는 없다.
	Super::BeginPlay();

	LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		if (UADNameWidget* NameWidget = Cast<UADNameWidget>(UserWidget))
		{
			NameWidget->SetNameText(NameText);
		}
	}

	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		UserWidget->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UNameWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!LocalPlayerController || !bIsEnabled)
	{
		return;
	}
	
	UpdateDistanceVisibility();
	UpdateBillboardRotation();
}

void UNameWidgetComponent::UpdateBillboardRotation()
{
	// 현재 위젯이 표시되고 있지 않으면 회전하지 않음
	if (!bIsVisible)
	{
		return;
	}

	// 가시성 체크는 Actor 기준으로 처리해야 하지만 위젯 각도는 컴포넌트의 위치를 기준으로 처리
	APlayerCameraManager* PlayerCameraManager = LocalPlayerController->PlayerCameraManager;
	FRotator BillboardRotation = GetComponentRotation();

	if (BillboardRotationMode == EBillboardRotationMode::LookAtCamera)
	{
		const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
		const FVector ComponentLocation = GetComponentLocation();
		BillboardRotation = (CameraLocation - ComponentLocation).Rotation();
	}
	else if (BillboardRotationMode == EBillboardRotationMode::ReverseCameraForward)
	{
		FVector CameraDirection = PlayerCameraManager->GetCameraRotation().Vector();
		BillboardRotation = (-CameraDirection).Rotation();
	}
	
	SetWorldRotation(BillboardRotation);
}

// PreCondition : bEnabled == true
void UNameWidgetComponent::UpdateDistanceVisibility()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(LocalPlayerController))
	{
		SetVisibility(false);
		return;
	}

	FVector LookTargetLocation;
	if (LocalPlayerController->GetPawn())
	{
		LookTargetLocation = LocalPlayerController->GetPawn()->GetActorLocation();
	}
	else if (LocalPlayerController->PlayerCameraManager)
	{
		LookTargetLocation = LocalPlayerController->PlayerCameraManager->GetCameraLocation();
	}
	else
	{
		SetVisibility(false);
		return;
	}

	float Distance = FVector::Distance(Owner->GetActorLocation(), LookTargetLocation);

	// 비활성화 상태에서 DisplayDistance 안으로 들어오면 이름 표시
	if (Distance <= DisplayDistance && !bIsVisible)
	{
		SetVisibility(true);
	}
	// 활성화 상태에서 MaxDisplayDistance 밖으로 나가면 이름 숨김
	else if (Distance > MaxDisplayDistance && bIsVisible)
	{
		SetVisibility(false);
	}
}

void UNameWidgetComponent::SetVisibility(bool bNewVisibility)
{
	if (bNewVisibility == bIsVisible)
	{
		return;
	}

	bIsVisible = bNewVisibility;

	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		UserWidget->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UNameWidgetComponent::SetNameText(const FString& NewName)
{
	if (NewName == NameText)
	{
		return;
	}

	NameText = NewName;

	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		if (UADNameWidget* NameWidget = Cast<UADNameWidget>(UserWidget))
		{
			NameWidget->SetNameText(NameText);
		}
	}
}

void UNameWidgetComponent::SetEnable(bool bNewEnabled)
{
	if (bNewEnabled == bIsEnabled)
	{
		return;
	}

	bIsEnabled = bNewEnabled;

	// 위젯이 비활성화되면 위젯도 숨김처리
	// 활성화되면 Tick에서 가시성 여부를 판단한다.
	SetVisibility(bIsEnabled && bIsVisible);
}

