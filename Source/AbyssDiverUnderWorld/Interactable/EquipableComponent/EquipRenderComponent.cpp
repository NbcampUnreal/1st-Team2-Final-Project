#include "Interactable/EquipableComponent/EquipRenderComponent.h"
#include "EquipRenderComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "EquipableComponent.h"
#include "Character/UnderwaterCharacter.h"

UEquipRenderComponent::UEquipRenderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    
}

void UEquipRenderComponent::AttachItem(AActor* RawItem, FName SocketName)
{
	if (!RawItem) return;

	if (ActiveEntries.Contains(RawItem)) return;

	UEquipableComponent* EComp = FindEquipableComponent(RawItem);
	if (!EComp)
	{
		LOG(TEXT("AttachItem 실패: %s에 EquipableComponent 없음"), *GetNameSafe(RawItem));
		return;
	}
	bool bIsSkeletal = EComp->IsSkeletal();
	bool bIsStatic = EComp->IsStatic();
	if (!bIsSkeletal && !bIsStatic)
	{
		LOG(TEXT("%s: 메시 타입을 알 수 없음."), *GetNameSafe(RawItem));
		return;
	}

	ACharacter* OwningChar = Cast<ACharacter>(GetOwner());
	if (!OwningChar)
	{
		return;
	}

	UMeshComponent* Mesh1P = Create1PComponent(EComp, OwningChar, SocketName);

	if (!Mesh1P)
	{
        LOG(TEXT("No Mesh1P"));
		return;
	}
    

	UMeshComponent* Mesh3P = Create3PComponent(EComp, OwningChar, SocketName);
	if (!Mesh3P)
	{
        LOG(TEXT("No Mesh3P"));
		Mesh1P->DestroyComponent();
		return;
	}
    


	ActiveEntries.Add(RawItem, { Mesh1P, Mesh3P });
} 

void UEquipRenderComponent::DetachItem(AActor* RawItem)
{
	if (!RawItem) return;

	if (ActiveEntries.Contains(RawItem))
	{
		const FRenderEntry& Entry = ActiveEntries[RawItem];

		if (Entry.Mesh1P.IsValid())
		{
			Entry.Mesh1P->DestroyComponent();
		}
		if (Entry.Mesh3P.IsValid())
		{
			Entry.Mesh3P->DestroyComponent();
		}
		ActiveEntries.Remove(RawItem);
	}
}

UEquipableComponent* UEquipRenderComponent::FindEquipableComponent(AActor* RawItem)
{
	if (!RawItem) return nullptr;
	return Cast<UEquipableComponent>(RawItem->GetComponentByClass(UEquipableComponent::StaticClass()));
}

UMeshComponent* UEquipRenderComponent::Create1PComponent(UEquipableComponent* EComp, ACharacter* OwningChar, FName SocketName)
{
	if (!EComp || !OwningChar) return nullptr;
    UMeshComponent* AnimSource = EComp->GetMeshComponent();
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(OwningChar);
    if (!AnimSource || !UnderwaterCharacter) return nullptr;
	
    if (EComp->IsSkeletal())
    {
        USkeletalMeshComponent* SkelSource = Cast<USkeletalMeshComponent>(AnimSource);
        USkeletalMesh* MeshSkelAsset = SkelSource->SkeletalMesh;
        if (!MeshSkelAsset) return nullptr;

        USkeletalMeshComponent* Skel1P = NewObject<USkeletalMeshComponent>(GetOwner());
        Skel1P->RegisterComponent();
        Skel1P->SetSkeletalMesh(MeshSkelAsset);
        Skel1P->AttachToComponent(
            UnderwaterCharacter->GetMesh1P(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            SocketName);
        Skel1P->SetOnlyOwnerSee(true);
        Skel1P->SetCastShadow(false);
        Skel1P->bCastHiddenShadow = false;

        LOG(
            TEXT("[AttachItem] Mesh1P=%s  |  Owner=%s  |  Outer=%s"),
            *GetNameSafe(Skel1P),
            *GetNameSafe(Skel1P->GetOwner()),
            *GetNameSafe(Skel1P->GetOuter()));
        AActor* VT = GetWorld()->GetFirstPlayerController()->GetViewTarget();
        LOG(TEXT("ViewTarget = %s"), *GetNameSafe(VT));

        return Skel1P;
    }
    else // Static
    {
        UStaticMeshComponent* StatSource = Cast<UStaticMeshComponent>(AnimSource);
        UStaticMesh* MeshStatAsset = StatSource->GetStaticMesh();
        if (!MeshStatAsset) return nullptr;

        UStaticMeshComponent* Stat1P = NewObject<UStaticMeshComponent>(GetOwner());
        Stat1P->RegisterComponent();
        Stat1P->SetStaticMesh(MeshStatAsset);
        Stat1P->AttachToComponent(
            UnderwaterCharacter->GetMesh1P(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            SocketName);
        Stat1P->SetOnlyOwnerSee(true);
        Stat1P->SetCastShadow(false);
        Stat1P->bCastHiddenShadow = false;

        LOG(
            TEXT("[AttachItem] Mesh1P=%s  |  Owner=%s  |  Outer=%s"),
            *GetNameSafe(Stat1P),
            *GetNameSafe(Stat1P->GetOwner()),
            *GetNameSafe(Stat1P->GetOuter()));
        AActor* VT = GetWorld()->GetFirstPlayerController()->GetViewTarget();
        LOG(TEXT("ViewTarget = %s"), *GetNameSafe(VT));

        return Stat1P;
    }
}

UMeshComponent* UEquipRenderComponent::Create3PComponent(UEquipableComponent* EComp, ACharacter* OwningChar, FName SocketName)
{

    if (!EComp || !OwningChar) return nullptr;
    UMeshComponent* AnimSource = EComp->GetMeshComponent();
    AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(OwningChar);
    if (!AnimSource)
        LOGN(TEXT("Attach FAIL: AnimSource is null for %s"), *GetNameSafe(EComp->GetOwner()));
    
    if (!AnimSource || !UnderwaterCharacter) return nullptr;

    if (EComp->IsSkeletal())
    {
        USkeletalMeshComponent* SkelSource = Cast<USkeletalMeshComponent>(AnimSource);
        USkeletalMesh* MeshSkelAsset = SkelSource->SkeletalMesh;
        if (!MeshSkelAsset) return nullptr;

        USkeletalMeshComponent* Skel3P = NewObject<USkeletalMeshComponent>(GetOwner());
        Skel3P->RegisterComponent();
        Skel3P->SetSkeletalMesh(MeshSkelAsset);
        Skel3P->AttachToComponent(
            UnderwaterCharacter->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            SocketName);
        Skel3P->SetOwnerNoSee(true);
        Skel3P->SetOnlyOwnerSee(false);
        Skel3P->MarkRenderStateDirty();
        Skel3P->SetCastShadow(true);
        Skel3P->bCastHiddenShadow = true;
        Skel3P->SetMasterPoseComponent(SkelSource);

        LOG(
            TEXT("[AttachItem] Mesh3P=%s  |  Owner=%s  |  Outer=%s"),
            *GetNameSafe(Skel3P),
            *GetNameSafe(Skel3P->GetOwner()),
            *GetNameSafe(Skel3P->GetOuter()));
        AActor* VT = GetWorld()->GetFirstPlayerController()->GetViewTarget();
        LOG(TEXT("ViewTarget = %s"), *GetNameSafe(VT));

        return Skel3P;
    }
    else // Static
    {
        UStaticMeshComponent* StatSource = Cast<UStaticMeshComponent>(AnimSource);
        UStaticMesh* MeshStatAsset = StatSource->GetStaticMesh();
        if (!MeshStatAsset) return nullptr;

        UStaticMeshComponent* Stat3P = NewObject<UStaticMeshComponent>(GetOwner());
        Stat3P->RegisterComponent();
        Stat3P->SetStaticMesh(MeshStatAsset);
        Stat3P->AttachToComponent(
            UnderwaterCharacter->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            SocketName);
        Stat3P->SetOwnerNoSee(true);
        Stat3P->MarkRenderStateDirty();
        Stat3P->SetCastShadow(true);
        Stat3P->bCastHiddenShadow = true;
        LOG(
            TEXT("[AttachItem] Mesh3P=%s  |  Owner=%s  |  Outer=%s"),
            *GetNameSafe(Stat3P),
            *GetNameSafe(Stat3P->GetOwner()),
            *GetNameSafe(Stat3P->GetOuter()));
        AActor* VT = GetWorld()->GetFirstPlayerController()->GetViewTarget();
        LOG(TEXT("ViewTarget = %s"), *GetNameSafe(VT));

        return Stat3P;
    }
}

