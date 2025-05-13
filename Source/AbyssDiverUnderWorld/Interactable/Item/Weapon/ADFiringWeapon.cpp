// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/Weapon/ADFiringWeapon.h"
#include "Projectile/ADProjectileBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

AADFiringWeapon::AADFiringWeapon()
{
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
}

void AADFiringWeapon::Fire()
{
	if (SkeletalMesh)
	{
		FTransform SpawnTransform = SkeletalMesh->GetSocketTransform(TEXT("FireLocationSocket"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = Cast<APawn>(GetOwner());
		SpawnParams.Owner = this;

		AADProjectileBase* Bullet = GetWorld()->SpawnActor<AADProjectileBase>(BulletClass, SpawnTransform, SpawnParams);
	}
}
