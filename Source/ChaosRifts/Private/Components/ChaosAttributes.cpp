// Copyright Robinator Studios, Inc. All Rights Reserved.

#include "Components/ChaosAttributes.h"

UChaosAttributes::UChaosAttributes()
{
	// This component does not need to tick every frame by itself.
	PrimaryComponentTick.bCanEverTick = false;
}

void UChaosAttributes::BeginPlay()
{
	Super::BeginPlay();

	// Initialize attributes to their max values at the start of the game.
	// This ensures that editing MaxHealth in a Blueprint correctly sets the starting Health.
	Health = MaxHealth;
	Chaos = MaxChaos;
	HealCharges = MaxHealCharges;
}

void UChaosAttributes::ApplyHealthChange(float Delta)
{
	const float OldHealth = Health;
	// Use FMath::Clamp to ensure Health never goes below 0 or above MaxHealth.
	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);
	
	if (OldHealth != Health)
	{
		// Log the change for debugging purposes.
		UE_LOG(LogTemp, Log, TEXT("Actor '%s' health changed from %f to %f (Delta: %f)"), *GetOwner()->GetName(), OldHealth, Health, Delta);

		if (Health == 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor '%s' has died!"), *GetOwner()->GetName());
			// In the future, a delegate (OnHealthDepleted) would be broadcast here.
		}
	}
}

void UChaosAttributes::ApplyChaosChange(float Delta)
{
	const float OldChaos = Chaos;
	Chaos = FMath::Clamp(Chaos + Delta, 0.0f, MaxChaos);
	
	if (OldChaos != Chaos)
	{
		UE_LOG(LogTemp, Log, TEXT("Actor '%s' chaos changed from %f to %f (Delta: %f)"), *GetOwner()->GetName(), OldChaos, Chaos, Delta);
	}
}

void UChaosAttributes::ApplyHealChargeChange(int32 Delta)
{
	const int32 OldCharges = HealCharges;
	HealCharges = FMath::Clamp(HealCharges + Delta, 0, MaxHealCharges);

	if (OldCharges != HealCharges)
	{
		UE_LOG(LogTemp, Log, TEXT("Actor '%s' heal charges changed from %d to %d (Delta: %d)"), *GetOwner()->GetName(), OldCharges, HealCharges, Delta);
	}
}
