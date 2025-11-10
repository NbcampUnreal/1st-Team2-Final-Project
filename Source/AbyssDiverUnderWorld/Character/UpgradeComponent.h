#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataRow/UpgradeDataRow.h"
#include "UpgradeComponent.generated.h"

// Network 연동은 현재 고려하지 않는다.
// 이는 Shop에서 어떻게 처리하는지에 따라 달라지기 때문이다.
// Shop에서 Upgrade RPC를 이용해서 처리하고 Shop의 정보가 갱신이 된다고 해도
// Client에서는 Upgrade 정보가 Replicate되는데 시간이 걸린다.
// 이 사이의 시간에서는 Server, Client 분단이 발생하게 되며 이 부분을 해결하는 구조를 선택해야 한다.
// 즉, 단순한 Replicate 로는 동기화 이슈를 해결할 수 없다.
// 문의한 결과 그러한 불일치성은 단기간에 발생하며 Server에서 검증하기 떄문에 허용 가능하는 것으로 결정되었다.

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUpgradeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method

public:

	UFUNCTION(Server, Unreliable)
	void S_RequestUpgrade(EUpgradeType UpgradeType);
	void S_RequestUpgrade_Implementation(EUpgradeType UpgradeType);

protected:
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUpgradePerformed"))
	void K2_OnUpgradePerformed(EUpgradeType UpgradeType, uint8 Grade);

private:

	UFUNCTION()
	void OnRep_UpgradeGradeMap();

#pragma endregion
	
#pragma region Variable
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradePerformed, EUpgradeType, UpgradeType, uint8, Grade);
	/** Upgrade가 되었을 경우 호출되는 Delegate */
	UPROPERTY(BlueprintAssignable)
	FOnUpgradePerformed OnUpgradePerformed;

	virtual void CopyProperties(UUpgradeComponent* Other);
	
private:
	/** Data Table Subsystem Weak Pointer */
	TWeakObjectPtr<class UDataTableSubsystem> DataTableSubsystem;
	
	/** 현재 Upgrade 정보를 저장한다. Type 별로 Grade를 저장하며 1이 기본값이다. */
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_UpgradeGradeMap)
	TArray<uint8> UpgradeGradeMap;
	
#pragma endregion

#pragma region Getter Setter

public:
	
	/** Upgrade Type에 해당하는 Grade를 반환, Grade는 1부터 시작하며 잘못된 입력을 할 경우 0을 반환 */
	UFUNCTION(BlueprintCallable)
	uint8 GetGradeByType(EUpgradeType UpgradeType) const;

	/** Upgrade Type에 해당하는 Grade를 1 증가, 최대 레벨일 경우 false를 반환 */
	UFUNCTION(BlueprintCallable)
	bool Upgrade(EUpgradeType UpgradeType);
	
	/** Upgrade Type의 Grade를 지정, 최대 레벨을 벗어날 경우 false를 반환 */
	UFUNCTION(BlueprintCallable)
	bool TrySetCurrentGrade(EUpgradeType UpgradeType, uint8 Grade);

	/** 현재 Type을 Upgrade하기 위한 비용을 반환, 최대 레벨일 경우 음수를 반환 */
	UFUNCTION(BlueprintCallable)
	int32 GetUpgradeCost(EUpgradeType UpgradeType) const;

	/** Upgrade Type이 최대 레벨인지 확인, 다음 Grade가 없으면 Max Level이다. */
	UFUNCTION(BlueprintCallable)
	bool IsMaxGrade(EUpgradeType UpgradeType) const;

	/** 현재 Upgrade 정보를 가져온다. Type 별로 Grade를 저장하고 있고 1이 기본값이다. */
	const TArray<uint8>& GetUpgradeGradeMap() const;

protected:
	
#pragma  endregion
	
};
