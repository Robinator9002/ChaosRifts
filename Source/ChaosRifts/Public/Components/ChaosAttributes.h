// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChaosAttributes.generated.h"

/**
 * Manages all gameplay-relevant attributes for a character, such as Health and Chaos.
 * This component can be attached to any actor to give it attributes.
 * It is designed to be the single source of truth for all vitals and resources.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHAOSRIFTS_API UChaosAttributes : public UActorComponent
{
	GENERATED_BODY()

public:	
	UChaosAttributes();

	//~==============================================================================================
	//~ Getters - Functions to safely read attribute values from anywhere.
	//~==============================================================================================

	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	float GetHealth() const { return Health; }
	
	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	float GetChaos() const { return Chaos; }

	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	float GetMaxChaos() const { return MaxChaos; }
	
	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	int32 GetHealCharges() const { return HealCharges; }

	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	int32 GetMaxHealCharges() const { return MaxHealCharges; }

	//~==============================================================================================
	//~ Attribute Modifiers - Public functions to change attribute values.
	//~==============================================================================================

	/**
	 * Modifies the current Health by a given delta. Can be positive (healing) or negative (damage).
	 * Handles clamping between 0 and MaxHealth.
	 * @param Delta The amount to change health by.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	void ApplyHealthChange(float Delta);
	
	/**
	 * Modifies the current Chaos by a given delta. Can be positive (gaining) or negative (spending).
	 * Handles clamping between 0 and MaxChaos.
	 * @param Delta The amount to change chaos by.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	void ApplyChaosChange(float Delta);

	/**
	 * Modifies the current Heal Charges by a given delta. Can be positive (gaining) or negative (using).
	 * Handles clamping between 0 and MaxHealCharges.
	 * @param Delta The amount to change charges by.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Attributes")
	void ApplyHealChargeChange(int32 Delta);


protected:
	virtual void BeginPlay() override;

	//~==============================================================================================
	//~ Attributes - The actual data properties. EditDefaultsOnly allows setting base values in Blueprints.
	//~==============================================================================================

	/** The current health of the character. Is initialized to MaxHealth in BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Attributes")
	float Health;

	/** The maximum health of the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chaos|Attributes", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/** The current chaos (mana/resource) of the character. Is initialized to MaxChaos in BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Attributes")
	float Chaos;

	/** The maximum chaos (mana/resource) of the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chaos|Attributes")
	float MaxChaos = 100.f;

	/** The current number of heal charges (potions). Is initialized to MaxHealCharges in BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Attributes")
	int32 HealCharges;

	/** The maximum number of heal charges (potions). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chaos|Attributes", meta = (ClampMin = "0"))
	int32 MaxHealCharges = 3;
};
