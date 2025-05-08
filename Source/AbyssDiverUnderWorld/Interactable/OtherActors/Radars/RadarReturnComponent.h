#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RadarReturnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class ABYSSDIVERUNDERWORLD_API URadarReturnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	URadarReturnComponent();

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Methods

public:

	// 아군용 리턴 크기 조절
	void ChangeFriendlyReturnSize(float NewSize);
	// 적군용 리턴 크기 조절
	void ChangeHostileReturnSize(float NewSize);
	// 중립용 리턴 크기 조절
	void ChangeNeutralReturnSize(float NewSize);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	TObjectPtr<USceneComponent> AppliesToComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	TObjectPtr<USceneComponent> ScaleFromComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UStaticMesh> FriendlyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UMaterialInterface> FriendlyMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	float FriendlyMeshScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UStaticMesh> HostileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UMaterialInterface> HostileMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	float HostileMeshScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UStaticMesh> NeutralMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	TObjectPtr<UMaterialInterface> NeutralMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization");
	float NeutralMeshScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Return Mesh Customization")
	uint8 bShouldLimitOpacityToRadarDisplay : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization")
	uint8 bHasReturnStand : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UStaticMesh> FriendlyStandMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UMaterialInterface> FriendlyStandMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UStaticMesh> HostileStandMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UMaterialInterface> HostileStandMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UStaticMesh> NeutralStandMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization");
	TObjectPtr<UMaterialInterface> NeutralStandMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization")
	float StandXYScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization")
	float StandZScaleOffset = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Stand Mesh Customization")
	uint8 bShouldLimitOpacityToRadarDisplayForStand : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization")
	uint8 bHasReturnPing : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UStaticMesh> FriendlyPingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UMaterialInterface> FriendlyPingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UStaticMesh> HostilePingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UMaterialInterface> HostilePingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UStaticMesh> NeutralPingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization");
	TObjectPtr<UMaterialInterface> NeutralPingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization")
	float PingScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|Ping Mesh Customization")
	uint8 bShouldLimitOpacityToRadarDisplayForPing : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|RangeModifiers")
	float VisibleRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|RangeModifiers")
	float FriendlyVisibleRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|RangeModifiers")
	float HostileVisibleRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn|RangeModifiers")
	float NeutralVisibleRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	FString ApplyToComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	TArray<FName> FactionTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	TArray<FName> UnitsTypeTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	uint8 bIsActivated : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	uint8 bIsPrimaryReturn : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadarReturn")
	FString ScaleFromRootComponentName;

#pragma endregion

#pragma region Getters, Setters

public:

	UStaticMesh* GetFriendlyMesh() const;
	UStaticMesh* GetHostileMesh() const;
	UStaticMesh* GetNeutralMesh() const;

	UMaterialInterface* GetFriendlyMaterial() const;
	UMaterialInterface* GetHostileMaterial() const;
	UMaterialInterface* GetNeutralMaterial() const;

	UStaticMesh* GetFriendlyStandMesh() const;
	UStaticMesh* GetHostileStandMesh() const;
	UStaticMesh* GetNeutralStandMesh() const;

	UMaterialInterface* GetFriendlyStandMaterial() const;
	UMaterialInterface* GetHostileStandMaterial() const;
	UMaterialInterface* GetNeutralStandMaterial() const;

	UStaticMesh* GetFriendlyPingMesh() const;
	UStaticMesh* GetHostilePingMesh() const;
	UStaticMesh* GetNeutralPingMesh() const;

	UMaterialInterface* GetFriendlyPingMaterial() const;
	UMaterialInterface* GetHostilePingMaterial() const;
	UMaterialInterface* GetNeutralPingMaterial() const;

	bool IsPrimaryReturn() const;
	void SetIsPrimaryReturn(bool bInIsPrimaryReturn);

	bool IsActivated() const;

	const TArray<FName>& GetFactionTags() const;
	const TArray<FName>& GetUnitsTypeTags() const;

	float GetVisibleRangeMultiplier() const;
	float GetFriendlyVisibleRangeMultiplier() const;
	float GetHostileVisibleRangeMultiplier() const;
	float GetNeutralVisibleRangeMultiplier() const;

	TObjectPtr<USceneComponent> GetAppliesToComponent() const;
	TObjectPtr<USceneComponent> GetScaleFromComponent() const;

	float GetFriendlyMeshScale() const;
	float GetHostileMeshScale() const;
	float GetNeutralMeshScale() const;
	float GetStandXYScaleMultiplier() const;
	float GetPingScaleMultiplier() const;
	float GetStandZScaleOffset() const;

	bool ShouldLimitOpacityToRadarDisplay() const;
	bool HasReturnStand() const;
	bool ShouldLimitOpacityToRadarDisplayForStand() const;
	bool HasReturnPing() const;
	bool ShouldLimitOpacityToRadarDisplayForPing() const;

#pragma endregion
};
