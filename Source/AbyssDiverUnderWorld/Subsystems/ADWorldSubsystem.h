#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "ADWorldSubsystem.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:

	void SetVirtualShadowMapsEnabled(bool bShouldEnable);
	void SafeEnableVSMDeferred();
	void SafeDisableVSM();

private:

	static const FString MainMenuLevelName;
	static const FString CampLevelName;
	static const FString ShallowLevelName;
	static const FString DeepLevelName;
	FString CurrentLevelName;

	FTimerHandle EnableTimerHandle;
	FTimerHandle RestoreTimerHandle;

public:
	FString GetCurrentLevelName() const { return CurrentLevelName; }
};
