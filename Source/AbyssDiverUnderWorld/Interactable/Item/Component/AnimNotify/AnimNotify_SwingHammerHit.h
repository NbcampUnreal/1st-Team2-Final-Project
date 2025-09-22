#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SwingHammerHit.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_SwingHammerHit : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region Method
public:
    UAnimNotify_SwingHammerHit();

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

#pragma endregion

#pragma region Variable
public:
    UPROPERTY(EditAnywhere, Category = "Hammer")
    float Damage = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Hammer")
    float Range = 140.f;

    UPROPERTY(EditAnywhere, Category = "Hammer")
    float Radius = 60.f;

    UPROPERTY(EditAnywhere, Category = "Hammer")
    TEnumAsByte<ECollisionChannel> MonsterChannel = ECC_GameTraceChannel3;

    // 보스 면역 옵션 -> 나중에 태그를 추가해서 로직 적용
    UPROPERTY(EditAnywhere, Category = "Hammer")
    bool bIgnoreBoss = true;

    UPROPERTY(EditAnywhere, Category = "Hammer", meta = (EditCondition = "bIgnoreBoss"))
    FName BossTag = TEXT("Boss");

    // 디버그 유무
    UPROPERTY(EditAnywhere, Category = "Hammer|Debug")
    bool bDebugDraw = false;
};
