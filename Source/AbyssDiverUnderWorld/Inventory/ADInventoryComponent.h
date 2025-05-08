// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Container/FStructContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ADInventoryComponent.generated.h"


enum class EItemType : uint8;

DECLARE_MULTICAST_DELEGATE_OneParam(FInventoryUpdateDelegate, EItemType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UADInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UADInventoryComponent();
protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; //FastArray 관련 함수

	UFUNCTION(BlueprintCallable)
	void AddInventoryItem(FName Name, uint8 Count);

	//*Remove 테스트 후 넣어야 함
	//*Wiget 관련 함수 넣어야 함

private:
	int8 GetTypeInventoryEmptyIndex(EItemType ItemType);
	int16 FindItemIndexById(FName ItemID);
	void PrintLogInventoryData();

	FInventoryUpdateDelegate InventoryUpdateDelegate;
#pragma endregion
	
#pragma region Variable
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItemDataTable; //*테스트용 추후 삭제
	//*Wiget 관련 변수 넣어야 함

private:
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	int16 TotalWeight;
	int16 WeightMax;
	uint8 bInventoryWidgetShowed : 1;
	TMap<EItemType, TArray<int8>> InventoryIndexMapByType;
	TArray<int8> InventorySizeByType;
#pragma endregion


#pragma region Getter, Setter
public:
	int16 GetTotalWeight() const { return TotalWeight; }
	const FInventoryList& GetInventoryList() { return InventoryList; }
	const TArray<int8>& GetInventoryIndexesByType(EItemType ItemType) const { return InventoryIndexMapByType[ItemType]; }
	const TArray<int8>& GetInventorySizeByType() const { return InventorySizeByType; }
#pragma endregion
};
