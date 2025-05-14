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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> LowerBodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> WeakPointMesh;
	
};
