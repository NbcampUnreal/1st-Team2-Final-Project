// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/UseFunction/EnableShield.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

void UEnableShield::Use(AActor* Target)
{
	//AADPlayerState* PS = Cast<AADPlayerState>(Target);
	//if (PS)
	//{
	// 
	//}
	//캐릭터 쪽에서 bool이 있다면 그걸 여기서 활성화시키고 hp가 달면 비활성화시키는 걸 캐릭터쪽에서 해야할 것 같다.
}
