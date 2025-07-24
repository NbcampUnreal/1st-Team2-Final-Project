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
    Step08_LightToggle,
    Step8_Items,
    Step9_OxygenWarning,
    Step10_Revival,
    Complete
};

UENUM(BlueprintType)
enum class ETutorialHintKey : uint8
{
    None UMETA(DisplayName = "None"),
    Move UMETA(DisplayName = "Move"),
    Sprint UMETA(DisplayName = "Sprint"),
    Oxygen UMETA(DisplayName = "Oxygen"),
    Drone UMETA(DisplayName = "Drone"),
    Flashlight UMETA(DisplayName = "Flashlight"),
    Revive UMETA(DisplayName = "Revive"),
    Inventory UMETA(DisplayName = "Inventory")
};