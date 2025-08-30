#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DexBitsSaveGame.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UDexBitsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> SavedBits;

};
