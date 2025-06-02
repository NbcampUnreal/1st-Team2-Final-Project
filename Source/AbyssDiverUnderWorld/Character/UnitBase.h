#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "UnitBase.generated.h"

UENUM(BlueprintType)
enum class EUnitId : uint8
{
	PlayerCharacter,
	Max UMETA(Hidden)
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnitBase : public ACharacter
{
	GENERATED_BODY()

public:

	AUnitBase();

protected:

	virtual void BeginPlay() override;
	
#pragma region Method
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
#pragma endregion

#pragma region Variable

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stat")
	TObjectPtr<class UStatComponent> StatComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Id")
	EUnitId UnitId;

#pragma endregion

public:
	FORCEINLINE UStatComponent* GetStatComponent() const { return StatComponent; }
	FORCEINLINE const EUnitId& GetUnitId() const { return UnitId; }
};
