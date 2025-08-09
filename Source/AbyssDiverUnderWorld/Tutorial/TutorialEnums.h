// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETutorialPhase : uint8
{
	None,
	Dialogue_01,         
	Step1_Movement,      
	Step2_Sprint,        
	Step3_Oxygen,        
	Step4_Radar,         
	Dialogue_02,        
	Step5_Looting,    
	Step6_Inventory,     
	Step7_Drone,       
	Dialogue_03,        
	Step8_LightToggle,   
	Step9_Items,         
	Step10_OxygenWarning,
	Step11_Revival,      
	Dialogue_04,         
	Complete
};

UENUM(BlueprintType)
enum class ETutorialHintKey : uint8
{
	None UMETA(DisplayName = "None"),
	Move UMETA(DisplayName = "Move"),
	Sprint UMETA(DisplayName = "Sprint"),
	Oxygen UMETA(DisplayName = "Oxygen"),
	Radar UMETA(DisplayName = "Radar"),
	Looting UMETA(DisplayName = "Looting"),
	Inventory UMETA(DisplayName = "Inventory"),
	Drone UMETA(DisplayName = "Drone"),
	Flashlight UMETA(DisplayName = "Flashlight"),
	Items UMETA(DisplayName = "Items"), 
	Revive UMETA(DisplayName = "Revive")
};

UENUM(BlueprintType)
enum class ETutorialHighlightTarget : uint8
{
	None UMETA(DisplayName = "None"),
	OxygenBar UMETA(DisplayName = "Oxygen Bar"),
	OreTarget UMETA(DisplayName = "OreTarget"),
	SpearPanel UMETA(DisplayName = "Spear Panel"),
	RadarIcon UMETA(DisplayName = "Radar Icon"),
	InventoryButton UMETA(DisplayName = "Inventory Button"),
	FlashlightToggle UMETA(DisplayName = "Flashlight Toggle"),
	ReviveHintIcon UMETA(DisplayName = "Revive Hint Icon")

};

UENUM(BlueprintType)
enum class EPlayerActionTrigger : uint8
{
	None,         

	Sprint        UMETA(DisplayName = "Sprint (Shift)"),

	Interact      UMETA(DisplayName = "Interact (E)"),

	Radar         UMETA(DisplayName = "Radar (Q)"),

	Inventory     UMETA(DisplayName = "Inventory (Tab)"),

	Flashlight    UMETA(DisplayName = "Flashlight (F)"),

	Reload        UMETA(DisplayName = "Reload/Recharge (R)"),

	UseItem1      UMETA(DisplayName = "Use Item 1"),
	UseItem2      UMETA(DisplayName = "Use Item 2"),
	UseItem3      UMETA(DisplayName = "Use Item 3"),

	Ascend        UMETA(DisplayName = "Ascend (Space)"),
	Descend       UMETA(DisplayName = "Descend (C)")
};