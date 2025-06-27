// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/ChaosAttributes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/Weapons/Weapon.h" // Important: Include the new weapon class

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
	// Destroy old weapons if this function is called again
	for (AWeapon* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->Destroy();
		}
	}
	Weapons.Empty();
	CurrentWeapon = nullptr;
    CurrentWeaponIndex = -1; // -1 means no weapon is active

	if (DefaultWeapons.Num() > 0)
	{
		// We have valid weapon classes to spawn
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		for (TSubclassOf<AWeapon> WeaponClass : DefaultWeapons)
		{
			if (WeaponClass)
			{
				AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParams);
				if (NewWeapon)
				{
					Weapons.Add(NewWeapon);
					// Socket to which the weapon is attached
					NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
					NewWeapon->SetActorHiddenInGame(true); // Hide the weapon initially
				}
			}
		}

		if (Weapons.Num() > 0)
		{
			// We have spawned at least one weapon, so equip the first one.
			EquipWeapon(0);
		}
	}
}

void AChaosCharacterBase::EquipWeapon(int32 WeaponIndex)
{
	if (WeaponIndex < 0 || WeaponIndex >= Weapons.Num() || Weapons[WeaponIndex] == nullptr)
	{
		// Invalid index or no weapons available
		return;
	}

	// Hide the old weapon if one was present
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	CurrentWeapon = Weapons[WeaponIndex];
	CurrentWeaponIndex = WeaponIndex;
	CurrentWeapon->SetActorHiddenInGame(false); // Show the new weapon
}

void AChaosCharacterBase::StartAttack()
{
	// The logic for an attack would be implemented here.
	// For example: play animation, enable weapon collision etc.
	// UE_LOG(LogTemp, Warning, TEXT("Attack started by %s"), *GetNameSafe(this));
	if (CurrentWeapon)
	{
		// This will be expanded in the future, e.g. to switch the weapon to "aggressive" state.
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
