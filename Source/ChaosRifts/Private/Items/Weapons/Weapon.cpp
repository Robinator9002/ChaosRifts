// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Items/Weapons/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AWeapon::AWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	// Set the weapon mesh as the new root component, as it is the main visible element.
	// The SceneRoot from AItem is no longer needed here.
	RootComponent = WeaponMesh;

	// Initial state and damage
	CurrentWeaponState = EWeaponState::Passive;
	Damage = 25.f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Bind our handle function to the base class's overlap delegate.
	OnItemOverlap.AddDynamic(this, &AWeapon::HandleItemOverlap);
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	CurrentWeaponState = NewState;

	// When the weapon returns to the passive state, we clear the list of
	// hit actors so that the next attack is fresh.
	if (NewState == EWeaponState::Passive)
	{
		DamagedActorsInSwing.Empty();
	}
}

void AWeapon::HandleItemOverlap(AActor* OverlappedActor, bool bIsOverlapping)
{
	// We are only interested in beginning overlaps while the weapon is aggressive.
	if (CurrentWeaponState == EWeaponState::Aggressive && bIsOverlapping)
	{
		// Ensure the owner of the weapon is valid.
		AActor* MyOwner = GetOwner();
		if (!MyOwner)
		{
			return;
		}

		// Prevent hitting the same actor multiple times in the same swing.
		if (DamagedActorsInSwing.Contains(OverlappedActor))
		{
			return;
		}

		// Add the hit actor to the list.
		DamagedActorsInSwing.Add(OverlappedActor);

		// Apply damage.
		AController* InstigatorController = MyOwner->GetInstigatorController();
		UGameplayStatics::ApplyDamage(
			OverlappedActor,
			Damage,
			InstigatorController,
			this, // The damage causer is this weapon actor
			nullptr // The damage type class. Can be null.
		);
	}
}
