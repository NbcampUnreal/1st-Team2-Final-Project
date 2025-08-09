// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UseStrategy.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ABYSSDIVERUNDERWORLD_API UUseStrategy : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool Use(AActor* Target) PURE_VIRTUAL(UUseStrategy::Use, return true;);//PURE_VIRTUAL은 자식이 override 하도록 강제
};
