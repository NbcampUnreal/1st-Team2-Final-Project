// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETutorialPhase : uint8
{
    None,
    Step1_Movement,
    Step2_SprintAndOxygen,
    Step3_Radar,
    Step4_Looting,
    Step5_Drone,
    Step6_LightToggle,
    Step7_Items,
    Step8_OxygenWarning,
    Step9_Revival,
    Complete
};

UENUM(BlueprintType)
enum class ETutorialHintKey : uint8
{
    None UMETA(DisplayName = "None"),
    Move UMETA(DisplayName = "Move"),
    Sprint UMETA(DisplayName = "Sprint"),
    Flashlight UMETA(DisplayName = "Flashlight"),
    Revive UMETA(DisplayName = "Revive"),
    Inventory UMETA(DisplayName = "Inventory")
};