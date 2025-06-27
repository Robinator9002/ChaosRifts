// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Items/Weapons/Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AWeapon::AWeapon()
{
	// Initial state and damage
	CurrentWeaponState = EWeaponState::Passive;
	Damage = 25.f;
	bIgnoreOwner = true;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Bind our handle function to the mesh's overlap delegate.
	if (ItemMesh)
	{
		ItemMesh->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnMeshBeginOverlap);
	}
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

void AWeapon::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// We are only interested in overlaps while the weapon is aggressive.
	if (CurrentWeaponState != EWeaponState::Aggressive)
	{
		return;
	}

	AActor* MyOwner = GetOwner();

	// Check if we should ignore the owner
	if (bIgnoreOwner && OtherActor == MyOwner)
	{
		return;
	}

	// Prevent hitting the same actor multiple times in the same swing.
	if (DamagedActorsInSwing.Contains(OtherActor))
	{
		return;
	}

	// Check if the overlapped actor's class is in the ignored list.
    for (TSubclassOf<AActor> ClassToIgnore : IgnoredActorClasses)
    {
        if (OtherActor->IsA(ClassToIgnore))
        {
            return;
        }
    }


	// Ensure the owner of the weapon is valid before dealing damage.
	if (!MyOwner)
	{
		return;
	}

	// Add the hit actor to the list.
	DamagedActorsInSwing.Add(OtherActor);

	// Apply damage.
	AController* InstigatorController = MyOwner->GetInstigatorController();
	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		InstigatorController,
		this, // The damage causer is this weapon actor
		nullptr // The damage type class. Can be null.
	);
}
