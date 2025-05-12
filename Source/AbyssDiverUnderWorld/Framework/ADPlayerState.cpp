// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

AADPlayerState::AADPlayerState()
{
	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));

}
