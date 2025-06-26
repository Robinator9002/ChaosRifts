// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosCharacterBase.generated.h"

class UChaosAttributes;

// A delegate that is broadcast when a character dies.
// Can be subscribed to by other systems to react to death (e.g., UI updates, AI reactions).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AChaosCharacterBase*, DeadCharacter);

/**
 * The base class for all characters in ChaosRifts, including the player, enemies, and NPCs.
 * It contains shared functionality, most importantly the Attributes component, and basic combat logic.
 */
UCLASS(abstract)
class CHAOSRIFTS_API AChaosCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacterBase();

	/** Returns the Attributes component for easy access from other classes. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Character")
	FORCEINLINE UChaosAttributes* GetAttributes() const { return AttributesComponent; }

	//~==============================================================================================
	//~ Combat Interface - Basic combat and damage functions
	//~==============================================================================================

	/**
	 * Overrides the standard TakeDamage function from AActor.
	 * This is the central point where all incoming damage is processed.
	 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * Function called when this character dies (health falls to 0).
	 * This is a virtual function that can be overridden by derived classes
	 * to implement specific death animations, effects, or gameplay consequences.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Chaos|Combat")
	void Die();
	virtual void Die_Implementation(); // Implementation of the BlueprintNativeEvent function

	/**
	 * Starts an attack. This is a virtual function that can be overridden by derived classes
	 * to implement specific attack animations and logic.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat")
	virtual void StartAttack();

	// Delegate broadcast when this character dies.
	UPROPERTY(BlueprintAssignable, Category = "Chaos|Combat")
	FOnDeathDelegate OnDeath;

protected:
	/** The component that manages all gameplay attributes for this character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChaosAttributes> AttributesComponent;
};
