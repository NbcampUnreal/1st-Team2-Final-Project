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