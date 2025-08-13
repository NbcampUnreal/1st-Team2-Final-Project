#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RadarReturn2DComponent.generated.h"

UENUM()
enum class EReturnForceType : uint8
{
	Friendly,
	Hostile,
	Neutral
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API URadarReturn2DComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	URadarReturn2DComponent();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Variables
public:
	// 레이더에 표시할 지 말 지 설정, true이면 bAlwaysDisplay는 무시
	UPROPERTY(EditAnywhere, Category = "RadarReturnSettings")
	uint8 bAlwaysIgnore : 1 = false;
protected:

	UPROPERTY(EditAnywhere, Category = "RadarReturnSettings")
	float ReturnScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "RadarReturnSettings")
	EReturnForceType ReturnForceType = EReturnForceType::Hostile;

	// 레이더에 항상 표시할 지 말 지 설정
	UPROPERTY(EditAnywhere, Category = "RadarReturnSettings")
	uint8 bAlwaysDisplay : 1 = false;

#pragma endregion

#pragma region Getters / Setters

public:

	float GetReturnScale() const;
	void SetReturnScale(float NewScale);

	EReturnForceType GetReturnForceType() const;
	void SetReturnForceType(EReturnForceType NewReturnForceType);

	bool GetAlwaysDisplay() const;
	void SetAlwaysDisplay(bool bShouldAlwaysDisplay);

	bool GetAlwaysIgnore() const;
	void SetAlwaysIgnore(bool bShouldAlwaysIgnore);

#pragma endregion

};
