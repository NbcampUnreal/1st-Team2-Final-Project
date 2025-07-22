#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "UnitBase.generated.h"

class URadarReturnComponent;
class URadarReturn2DComponent;

UENUM(BlueprintType)
enum class EUnitId : uint8
{
	PlayerCharacter,
	GobleFish,
	HorrorCreature,
	MegaTentacle,
	Tentacle,
	Blowfish,
	Limadon,
	SeaObserver,
	MiniSerpmare,
	BigSerpmare,
	Kraken,
	AlienShark,
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
	virtual void Destroyed() override;
	
#pragma region Method

protected:
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
#pragma endregion

#pragma region Variable

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stat")
	TObjectPtr<class UStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Radar Settings")
	TObjectPtr<URadarReturnComponent> RadarReturnComponent;

	UPROPERTY(EditAnywhere, Category = "Radar Settings")
	TObjectPtr<URadarReturn2DComponent> RadarReturn2DComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Id")
	EUnitId UnitId;

#pragma endregion

public:
	FORCEINLINE UStatComponent* GetStatComponent() const { return StatComponent; }
	FORCEINLINE const EUnitId& GetUnitId() const { return UnitId; }
};
