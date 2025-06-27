// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Items/Weapons/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AWeapon::AWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	// Setze das Waffen-Mesh als das neue Root-Component, da es das sichtbare Hauptelement ist.
	// Das SceneRoot von AItem ist hier nicht mehr nötig.
	RootComponent = WeaponMesh;

	// Initialzustand und Schaden
	CurrentWeaponState = EWeaponState::Passive;
	Damage = 25.f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Binde unsere Handle-Funktion an das Overlap-Delegate der Basisklasse.
	OnItemOverlap.AddDynamic(this, &AWeapon::HandleItemOverlap);
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	CurrentWeaponState = NewState;

	// Wenn die Waffe in den passiven Zustand zurückkehrt, leeren wir die Liste der
	// getroffenen Actors, damit der nächste Angriff wieder frisch ist.
	if (NewState == EWeaponState::Passive)
	{
		DamagedActorsInSwing.Empty();
	}
}

void AWeapon::HandleItemOverlap(AActor* OverlappedActor, bool bIsOverlapping)
{
	// Wir interessieren uns nur für beginnende Overlaps, während die Waffe aggressiv ist.
	if (CurrentWeaponState == EWeaponState::Aggressive && bIsOverlapping)
	{
		// Stelle sicher, dass der Besitzer der Waffe gültig ist.
		AActor* MyOwner = GetOwner();
		if (!MyOwner)
		{
			return;
		}

		// Verhindere, dass wir denselben Actor im selben Schwung mehrmals treffen.
		if (DamagedActorsInSwing.Contains(OverlappedActor))
		{
			return;
		}

		// Füge den getroffenen Actor zur Liste hinzu.
		DamagedActorsInSwing.Add(OverlappedActor);

		// Verursache Schaden.
		AController* InstigatorController = MyOwner->GetInstigatorController();
		UGameplayStatics::ApplyDamage(
			OverlappedActor,
			Damage,
			InstigatorController,
			this, // Der DamageCauser ist die Waffe selbst
			UDamageType::StaticClass()
		);

		// Optional: Log für Debugging
		UE_LOG(LogTemp, Log, TEXT("Weapon '%s' hit '%s' for %f damage."), *GetName(), *OverlappedActor->GetName(), Damage);
	}
}
