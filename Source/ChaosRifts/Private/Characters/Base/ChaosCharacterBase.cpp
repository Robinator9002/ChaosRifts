// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/ChaosAttributes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/Weapons/Weapon.h" // Wichtig: Die neue Waffenklasse inkludieren

AChaosCharacterBase::AChaosCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AttributesComponent = CreateDefaultSubobject<UChaosAttributes>(TEXT("AttributesComponent"));
	CurrentWeaponIndex = -1; // -1 bedeutet, dass keine Waffe ausgerüstet ist
}

void AChaosCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	// Wir rufen das Spawnen der Waffen hier auf, damit jeder Character, der erbt,
	// automatisch seine Waffen bekommt.
	SpawnAndEquipWeapons();
}

void AChaosCharacterBase::SpawnAndEquipWeapons()
{
	// Zerstöre alte Waffen, falls diese Funktion erneut aufgerufen wird
	for (AWeapon* Weapon : Weapons)
	{
		if (Weapon)
		{
			Weapon->Destroy();
		}
	}
	Weapons.Empty();
	CurrentWeapon = nullptr;

	if (DefaultWeapons.Num() > 0)
	{
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
					// Waffe an den Charakter attachen. Wir verwenden hier einen Socket namens "WeaponSocket".
					// Du musst diesen Socket in deinem Charakter-Skelett erstellen!
					// Für mehrere Waffen könnten Sockets wie "WeaponPrimarySocket", "WeaponHolsterSocket_Back" etc. verwendet werden.
					// Für dieses erweiterbare System gehen wir von einem einzigen Ankerpunkt aus; das Hiding/Unhiding regelt die Darstellung.
					NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
					Weapons.Add(NewWeapon);
				}
			}
		}

		// Rüste die erste Waffe in der Liste standardmäßig aus.
		EquipWeapon(0);
	}
}

void AChaosCharacterBase::EquipWeapon(int32 WeaponIndex)
{
	if (!Weapons.IsValidIndex(WeaponIndex) || Weapons[WeaponIndex] == nullptr)
	{
		return;
	}

	// Verstecke die aktuell ausgerüstete Waffe, falls eine existiert
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
		CurrentWeapon->SetWeaponState(EWeaponState::Passive);
	}

	// Setze die neue Waffe als aktuell
	CurrentWeaponIndex = WeaponIndex;
	CurrentWeapon = Weapons[CurrentWeaponIndex];

	// Zeige die neue Waffe und setze ihren Zustand zurück
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(false);
		CurrentWeapon->SetWeaponState(EWeaponState::Passive);
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

void AChaosCharacterBase::StartAttack()
{
	// Die Logik hier wird nun spezifischer in den Child-Klassen sein,
	// da sie die Animationen steuern, die wiederum die Waffe "scharf schalten".
	UE_LOG(LogTemp, Log, TEXT("%s starts a generic attack."), *GetNameSafe(this));
}
