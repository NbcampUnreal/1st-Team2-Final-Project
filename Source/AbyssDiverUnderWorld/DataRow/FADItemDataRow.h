#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FADItemDataRow.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equipment = 0,
	Consumable = 1,
	Exchangable = 2,
	Bullet = 4,
	Max = 5 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EBulletType : uint8
{
	None = 0,
	BasicSpear = 1,
	BombSpear = 2,
	PoisonSpear = 3,
	Flare = 4,
	Max = 5 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FFADItemDataRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmoInMag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReserveAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Weight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Stackable : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag LeftTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag RKeyTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBulletType BulletType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Thumbnail;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UObject> UseFunction;
};
