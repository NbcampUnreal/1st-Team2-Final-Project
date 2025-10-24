#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionsOnHUDWidget.generated.h"

enum class EMissionType : uint8;

class UVerticalBox;
class UMissionEntryOnHUDWidget;
class UMissionSubsystem;
class AADInGameState;
class UMissionManagerComponent;

struct FMissionRuntimeState;

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
	virtual void NativeDestruct() override;
#pragma region Methods

public:

	// 클라에선 OnPossessedPawnChanged 호출 타이밍이 너무 빨라서 GS를 가져올 수 없다. 알아서 타이밍 조절 필요함.
	UFUNCTION(BlueprintCallable)
	void InitWidget();
	void SetVisible(bool bShouldVisible, int32 ElementIndex);
	void UpdateMission(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress, int32 Goal, bool bCompleted);
	void MissionStatesRefresh();

protected:

	void CreateAndAddEntry(UTexture2D* Image);
	void ChangeImage(UTexture2D* Image, int32 ElementIndex);
	void UpdateMissionEntryColor(bool bIsMissionCompleted, int32 ElementIndex);

private:
	FTimerHandle InitRetryHandle;
	void TryBindManager();
	bool IsManagerReady() const { return MissionManager != nullptr; }

	UFUNCTION() 
	void HandleStatesUpdated(const TArray<FMissionRuntimeState>& States);
	// (완료 즉시 토스트/색 변경 원하면)
	UFUNCTION() 
	void HandleMissionCompletedUI(EMissionType MissionType, int32 MissionIndex);

	// 인덱스로 엔트리 위젯 얻기
	UMissionEntryOnHUDWidget* GetEntryBySlotIndex(int8 SlotIndex) const;
	// 새 엔트리 생성 & 리스트에 붙이고 슬롯 인덱스 반환
	int8 CreateAndAttachEntry(EMissionType Type, uint8 MissionIndex);
	// DataRow(제목/설명/아이콘) 채우기
	void FillEntryStaticTexts(UMissionEntryOnHUDWidget* Entry, EMissionType Type, uint8 MissionIndex);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> MissionListBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMissionEntryOnHUDWidget> MissionEntryClass;

	// 미션 타입, 미션 인덱스 : 위젯 엔트리 인덱스
	TMap<TPair<EMissionType, uint8>, int8> MissionEntryMap;

private:

	UPROPERTY()
	TObjectPtr<UMissionSubsystem> MissionSubsystem;

	UPROPERTY()
	TObjectPtr<UMissionManagerComponent> MissionManager;

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
