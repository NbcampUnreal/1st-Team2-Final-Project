#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MissionsOnHUDWidget.generated.h"

enum class EMissionType : uint8;

class UVerticalBox;
class UMissionEntryOnHUDWidget;
class UMissionSubsystem;
class AADInGameState;

struct FActivatedMissionInfoList;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionsOnHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

#pragma region Methods

public:

	// 클라에선 OnPossessedPawnChanged 호출 타이밍이 너무 빨라서 GS를 가져올 수 없다. 알아서 타이밍 조절 필요함.
	void InitWiget();
	void SetVisible(bool bShouldVisible, int32 ElementIndex);
	void UpdateMission(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress);
	void Refresh();

protected:

	void CreateAndAddEntry(UTexture2D* Image);
	void ChangeImage(UTexture2D* Image, int32 ElementIndex);
	void UpdateMissionEntryColor(bool bIsMissionCompleted, int32 ElementIndex);

private:

	void OnMissionChanged(int32 ChangedIndex, const FActivatedMissionInfoList& ChangedValue);
	void OnMissionRemoved(int32 RemovedIndex, const FActivatedMissionInfoList& RemovedValue);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MissionListBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMissionEntryOnHUDWidget> MissionEntryClass;

	// 미션 타입, 미션 인덱스 : 위젯 엔트리 인덱스
	TMap<TPair<EMissionType, uint8>, int8> ContainedMissions;

private:

	UPROPERTY()
	TObjectPtr<UMissionSubsystem> MissionSubsystem;

	UPROPERTY()
	TObjectPtr<AADInGameState> InGameState;

#pragma endregion

#pragma region Getters / Setters

public:

	int32 GetElementCount() const;
	UMissionSubsystem* GetMissionSubsystem();
	AADInGameState* GetInGameState();

#pragma endregion
};
