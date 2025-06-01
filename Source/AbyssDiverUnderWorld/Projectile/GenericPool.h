// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolableItem.h"
#include "GenericPool.generated.h"

class APoolableItem; 
class AADSpearGunBullet;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AGenericPool : public AActor
{
	GENERATED_BODY()
	
public:	
	AGenericPool();

	template <typename T>
	void InitPool(const int8 Size, TSubclassOf<T> SpawnClass)
	{
		static_assert(std::is_base_of<APoolableItem, T>::value, "T must be a subclass of APoolableItem"); //타입 제약
		PoolSize = Size;
		PoolableClass = SpawnClass;
		DeActivatedBulletCount = PoolSize;
		for (int8 i = 0; i < PoolSize; ++i)
		{
			T* NewActor = GetWorld()->SpawnActor<T>(PoolableClass, FVector(0, 0, 0), FRotator::ZeroRotator);
			if (NewActor)
			{
				NewActor->SetProjectileId(i);
				NewActor->Deactivate();
				ObjectPool.Add(NewActor);
				LOGOP(Warning, TEXT("Bullet add to BulletPool"), PoolSize);
			}
			else
			{
				LOGOP(Warning, TEXT("Failed to spawn bullet actor."));
			}
		}
		LOGOP(Warning, TEXT("BulletPool is Initialized. PoolSize : %d"), ObjectPool.Num());
	}

	template <typename T>
	T* GetObject()
	{
		static_assert(std::is_base_of<APoolableItem, T>::value, "T must be a subclass of APoolableItem");
		for (APoolableItem* Object : ObjectPool)
		{
			if (!Object->GetIsActive())
			{
				Object->SetObjectPool(this);
				Object->Activate();
				DeActivatedBulletCount--;
				LOGOP(Warning, TEXT("Bullet is Activated. Bullet Id : %d"), Object->GetProjectileId());
				LOGOP(Warning, TEXT("Number of Bullets left : %d"), DeActivatedBulletCount);
				return Cast<T>(Object);
			}
		}

		LOGOP(Warning, TEXT("There are no more bullet. Add a new bullet"));
		T* NewObject = GetWorld()->SpawnActor<T>(PoolableClass);
		if (NewObject)
		{
			NewObject->Activate();
			ObjectPool.Add(NewObject);
			return NewObject;
		}

		return nullptr;
	}

	void ReturnObject()
	{
		DeActivatedBulletCount++;
	}

private:
	UPROPERTY(VisibleAnywhere, Category = "Pool")
	TArray<APoolableItem*> ObjectPool;

	TSubclassOf<APoolableItem> PoolableClass;

	UPROPERTY(EditAnywhere, Category = "Pool")
	int8 PoolSize;
	
	int8 DeActivatedBulletCount = 0;
	
};
