#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "DamageType_Stagger.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UDamageType_Stagger : public UDamageType
{
	GENERATED_BODY()
	
public:
	UDamageType_Stagger();

#pragma region Variable
public:
    /** 기본 경직 지속 시간(초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stagger")
    float DefaultStaggerDuration;

#pragma endregion


};
