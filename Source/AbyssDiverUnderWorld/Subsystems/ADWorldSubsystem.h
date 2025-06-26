#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "ADWorldSubsystem.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:

	static const FString MainMenuLevelName;
	static const FString CampLevelName;
	static const FString ShallowLevelName;
	static const FString DeepLevelName;
	FString CurrentLevelName;

public:
	FString GetCurrentLevelName() const { return CurrentLevelName; }
};
