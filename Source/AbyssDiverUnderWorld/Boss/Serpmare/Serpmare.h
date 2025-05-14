#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "Serpmare.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASerpmare : public ABoss
{
	GENERATED_BODY()

public:
	ASerpmare();

public:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> AppearAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> DisappearAnimation;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> LowerBodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> WeakPointMesh;
	
};
