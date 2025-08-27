// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebuffComponent.generated.h"

UENUM(BlueprintType)
enum class EDebuffType : uint8
{
	None        UMETA(DisplayName = "None"),
	Poison      UMETA(DisplayName = "Poison")
};

USTRUCT(BlueprintType)
struct FDebuffInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDebuffType DebuffType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;

	FDebuffInfo() : DebuffType(EDebuffType::None), Damage(0.0f) {}
	FDebuffInfo(EDebuffType InDebuffType, float InDamage) : DebuffType(InDebuffType), Damage(InDamage) {}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UDebuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDebuffComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Category = "Debuff")
	void SetDebuffState(FDebuffInfo NewStateInfo);
	UFUNCTION(Category = "Debuff")
	void ApplyDebuff(FDebuffInfo NewStateInfo);

	FTimerHandle PoisonDebuffTimerHandle;
	FTimerHandle PoisonClearTimerHandle;
	int8 PoisonDebuffTick;

	UFUNCTION(Category = "Debuff")
	EDebuffType GetDebuffState() const { return CurrentDebuffState; }

protected:
	UPROPERTY(Replicated)
	EDebuffType CurrentDebuffState;

	
};
