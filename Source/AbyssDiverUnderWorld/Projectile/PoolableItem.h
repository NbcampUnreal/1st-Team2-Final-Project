// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolableItem.generated.h"

#define LOGOP(Verbosity, Format, ...) UE_LOG(ObjectPoolLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(ObjectPoolLog, Log, All);

UCLASS()
class ABYSSDIVERUNDERWORLD_API APoolableItem : public AActor
{
	GENERATED_BODY()
	
public:	
	APoolableItem();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	

	virtual void Activate();
	virtual void Deactivate();

	bool GetIsActive() const { return bIsActive; }

protected:
	UPROPERTY(Replicated)
	int8 ProjectileId = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pool")
	TObjectPtr<class AGenericPool> ObjectPool = nullptr;
	bool bIsActive;
public:
	int8 GetProjectileId() const { return ProjectileId; }
	void SetProjectileId(int8 Id);

	void SetObjectPool(class AGenericPool* Pool);

};
