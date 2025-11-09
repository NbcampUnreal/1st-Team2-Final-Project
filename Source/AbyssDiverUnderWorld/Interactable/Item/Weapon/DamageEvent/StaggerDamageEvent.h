#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "UObject/Class.h"
#include "StaggerDamageEvent.generated.h"

class UDamageType_Stagger;

/** 경직을 위한 커스텀 DamageEvent */
USTRUCT()
struct FStaggerDamageEvent : public FDamageEvent
{
    GENERATED_BODY()

    FStaggerDamageEvent();
    

#pragma region Variable
    /** 경직 지속시간 (0 이하일 경우 DamageType 기본값 사용) */
    UPROPERTY()
    float Duration;
#pragma endregion
    


#pragma region Getter, Setteer
    /** 고유 타입 ID – 다른 FDamageEvent와 겹치지 않도록 3 지정 */
    enum { ClassID = 3 };
    virtual int32 GetTypeID() const override { return FStaggerDamageEvent::ClassID; }
    virtual bool IsOfType(int32 InID) const override
    {
        return InID == FStaggerDamageEvent::ClassID || FDamageEvent::IsOfType(InID);
    }
#pragma endregion
    
};