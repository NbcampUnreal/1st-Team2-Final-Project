#include "Interactable/Item/ADItemBase.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AADItemBase::AADItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AADItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void AADItemBase::Interact(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		HandlePickup(Cast<APawn>(InstigatorActor));
	}
}

void AADItemBase::HandlePickup(APawn* InstigatorPawn)
{
	if (!HasAuthority() || !InstigatorPawn) return;

	LOG(TEXT("Add to Inventory"));
	// TODO �κ��丮 �߰� ������ ȹ�� ȿ�� �߰�


	Destroy();
}

void AADItemBase::OnRep_ItemData()
{
	// TODO UI ������Ʈ
}

void AADItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADItemBase, ItemData);
}


