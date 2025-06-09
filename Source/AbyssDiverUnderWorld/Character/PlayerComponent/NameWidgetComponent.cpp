// Fill out your copyright notice in the Description page of Project Settings.


#include "NameWidgetComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/NameWidget.h"


UNameWidgetComponent::UNameWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetUsingAbsoluteRotation(true);
	SetWidgetSpace(EWidgetSpace::World);
	
	DisplayDistance = 1000.0f;
	MaxDisplayDistance = 1500.0f;
	bIsVisible = false;
	DrawSize = FIntPoint(200, 100);
}

void UNameWidgetComponent::BeginPlay()
{
	// WidgetComponent에서 InitWidget을 호출하고 있으므로 명시적으로 생성할 필요는 없다.
	Super::BeginPlay();

	LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		if (UNameWidget* NameWidget = Cast<UNameWidget>(UserWidget))
		{
			NameWidget->SetNameText(NameText);
		}
	}
	SetVisibility(bIsVisible);
}

void UNameWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!LocalPlayerController)
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
	FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	FVector ComponentLocation = GetComponentLocation();

	FRotator LookAtRotation = (CameraLocation - ComponentLocation).Rotation();
	SetWorldRotation(LookAtRotation);
}

void UNameWidgetComponent::UpdateDistanceVisibility()
{
	AActor* Owner = GetOwner();

	float Distance = FVector::Distance(Owner->GetActorLocation(), LocalPlayerController->GetPawn()->GetActorLocation());

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
		if (UNameWidget* NameWidget = Cast<UNameWidget>(UserWidget))
		{
			NameWidget->SetNameText(NameText);
		}
	}
}

