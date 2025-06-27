// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/ChaosAttributes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/Weapons/Weapon.h"

AChaosCharacterBase::AChaosCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AttributesComponent = CreateDefaultSubobject<UChaosAttributes>(TEXT("AttributesComponent"));
	CurrentWeaponIndex = -1; // -1 means no weapon is equipped
}

void AChaosCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	// We call the weapon spawning here so that every inheriting character
	// automatically gets their weapons.
	SpawnAndEquipWeapons();
}

void AChaosCharacterBase::SpawnAndEquipWeapons()
{
	// Destroy old weapon actors if this function is called again
	for (AWeapon* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->Destroy();
		}
	}
	Weapons.Empty();
	CurrentWeapon = nullptr;
    CurrentWeaponIndex = -1;

	// Proceed if we have a valid loadout configured
	if (DefaultWeaponLoadout.Num() > 0)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		// Iterate through the loadout configuration
		for (const FWeaponLoadoutInfo& LoadoutInfo : DefaultWeaponLoadout)
		{
			if (LoadoutInfo.WeaponClass)
			{
				// Spawn the weapon
				AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(LoadoutInfo.WeaponClass, SpawnParams);
				if (NewWeapon)
				{
					// Add the new weapon instance to our runtime array
					Weapons.Add(NewWeapon);
					// Attach the weapon to its designated "sheathed" or "holstered" socket
					AttachWeaponToSocket(NewWeapon, LoadoutInfo.SheathedSocketName);
					NewWeapon->SetActorHiddenInGame(true); // Hide the weapon initially
				}
			}
		}

		if (Weapons.Num() > 0)
		{
			// Equip the first weapon in the loadout by default
			EquipWeapon(0);
		}
	}
}

void AChaosCharacterBase::EquipWeapon(int32 WeaponIndex)
{
	// Check for valid index and that the weapon instance exists
	if (!Weapons.IsValidIndex(WeaponIndex) || Weapons[WeaponIndex] == nullptr)
	{
		return;
	}

	// --- UNEQUIP OLD WEAPON ---
	// If we already have a weapon equipped...
	if (CurrentWeapon && DefaultWeaponLoadout.IsValidIndex(CurrentWeaponIndex))
	{
		// Get the loadout info for the CURRENTLY equipped weapon
		const FWeaponLoadoutInfo& OldWeaponLoadout = DefaultWeaponLoadout[CurrentWeaponIndex];
		// Attach it back to its sheathed position
		AttachWeaponToSocket(CurrentWeapon, OldWeaponLoadout.SheathedSocketName);
		// Hide it
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	// --- EQUIP NEW WEAPON ---
	// Get the new weapon and its loadout info
	AWeapon* NewWeaponToEquip = Weapons[WeaponIndex];
	const FWeaponLoadoutInfo& NewWeaponLoadout = DefaultWeaponLoadout[WeaponIndex];

	// Update our state
	CurrentWeapon = NewWeaponToEquip;
	CurrentWeaponIndex = WeaponIndex;

	// Attach the new weapon to the hand/equipped socket
	AttachWeaponToSocket(CurrentWeapon, NewWeaponLoadout.EquippedSocketName);
	// Make it visible
	CurrentWeapon->SetActorHiddenInGame(false);
}

void AChaosCharacterBase::AttachWeaponToSocket(AWeapon* WeaponToAttach, const FName& SocketName)
{
	if (!WeaponToAttach || SocketName.IsNone())
	{
		return;
	}

	// Attachment rules - we want the weapon to be welded to the socket, ignoring its own transform.
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	// --- Smart Socket/Component Search ---
	// First, check if a USceneComponent with the given name exists on this character.
	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);
	USceneComponent* TargetComponent = nullptr;

	for (USceneComponent* SceneComp : SceneComponents)
	{
		if (SceneComp->GetFName() == SocketName)
		{
			TargetComponent = SceneComp;
			break;
		}
	}

	if (TargetComponent)
	{
		// We found a dedicated Scene Component, attach to it.
		WeaponToAttach->AttachToComponent(TargetComponent, AttachmentRules);
	}
	else
	{
		// Fallback: If no Scene Component was found, attach to a socket on the main skeletal mesh.
		WeaponToAttach->AttachToComponent(GetMesh(), AttachmentRules, SocketName);
	}
}


void AChaosCharacterBase::StartAttack()
{
	if (CurrentWeapon)
	{
		// Logic is now handled in AChaosCharacter via anim notifies
	}
}

void AChaosCharacterBase::SwapToNextWeapon()
{
	if (Weapons.Num() > 1)
	{
		const int32 NextWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
		EquipWeapon(NextWeaponIndex);
	}
}

void AChaosCharacterBase::SwapToPreviousWeapon()
{
	if (Weapons.Num() > 1)
	{
		const int32 PrevWeaponIndex = (CurrentWeaponIndex - 1 + Weapons.Num()) % Weapons.Num();
		EquipWeapon(PrevWeaponIndex);
	}
}

AWeapon* AChaosCharacterBase::GetCurrentWeapon() const
{
	return CurrentWeapon;
}


float AChaosCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (!AttributesComponent) return 0.0f;
	
	AttributesComponent->ApplyHealthChange(-ActualDamage);

	if (AttributesComponent->GetHealth() <= 0.0f)
	{
		Die();
	}
	return ActualDamage;
}

void AChaosCharacterBase::Die_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Character '%s' has died!"), *GetNameSafe(this));
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);

	OnDeath.Broadcast(this);
}
