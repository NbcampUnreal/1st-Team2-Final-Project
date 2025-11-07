#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FlipbookWidget.generated.h"

class UImage;
class UPaperFlipbook;

UENUM()
enum class EFlipbookType : uint8
{
	HorrorCreatureShallow = 0,
	LimadonShallow,
	MAX UMETA(Hidden)
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UFlipbookWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

public:

	// PlayTime이 0 이하일 경우 무한재생
	void PlayFlipbook(EFlipbookType PlayingFlipbook, bool bShouldLoop, float PlayTime = 0);
	void StopFlipbook();

protected:

	void PlayNextFrame();

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FrameImage;

	UPROPERTY(EditDefaultsOnly, Category = "FlipbookSettings")
	TMap<EFlipbookType, TObjectPtr<UPaperFlipbook>> PaperFlipbooks;
	
	int32 CurrentFrameIndex = 0;
	EFlipbookType CurrentFlipbookType = EFlipbookType::MAX;

private:

	TWeakObjectPtr<UPaperFlipbook> CurrentPaperFlipbbook = nullptr;

	FTimerHandle FlipAnimTimerHandle;

	float CachedInterval = 999.0f;
	float ElapsedTime = 0.0f;
	float CachedPlayTime = 0.0f;

#pragma region Getters / Setters
public:

	bool IsPlaying() const;

#pragma endregion

};
