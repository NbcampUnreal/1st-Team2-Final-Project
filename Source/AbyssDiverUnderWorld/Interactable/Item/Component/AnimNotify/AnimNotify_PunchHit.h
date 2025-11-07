#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PunchHit.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_PunchHit : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_PunchHit();


#pragma region Method
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

#pragma endregion

#pragma region Variable
public:
    UPROPERTY(EditAnywhere, Category = "Fist")
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, Category = "Fist")
    float Range = 100.f;

    UPROPERTY(EditAnywhere, Category = "Fist")
    float Radius = 40.f;

    // 경직 발생 여부 
    UPROPERTY(EditAnywhere, Category = "Fist|Stagger")
    uint8 bCauseStagger : 1 = true;

    // 타격 채널 (Pawn + 몬스터 커스텀 채널 허용)
    UPROPERTY(EditAnywhere, Category = "Fist")
    TEnumAsByte<ECollisionChannel> MonsterChannel = ECC_GameTraceChannel3;

    /* 추후 추가 할 요소
    

    // 보스 면역/태그 옵션 (SwingHammer Notify와 동일 패턴)
    UPROPERTY(EditAnywhere, Category = "Fist")
    bool bIgnoreBoss = true;

    UPROPERTY(EditAnywhere, Category = "Fist", meta = (EditCondition = "bIgnoreBoss"))
    FName BossTag = TEXT("Boss");
    */
    // Debug Draw
    UPROPERTY(EditAnywhere, Category = "Fist|Debug")
    bool bDebugDraw = false;
#pragma endregion


};
