// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemy/ChaosEnemy.h" // Inherit from AChaosEnemy
#include "ChaosEnemyMelee.generated.h"

class UAnimMontage;

/**
 * Base class for AI-controlled melee enemies.
 * Implements a basic melee attack suitable for AI.
 */
UCLASS(abstract)
class CHAOSRIFTS_API AChaosEnemyMelee : public AChaosEnemy
{
	GENERATED_BODY()
	
public:
	AChaosEnemyMelee();

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

	//~==============================================================================================
	//~ Combat - Overrides from AChaosCharacterBase for specific attack implementation
	//~==============================================================================================

	/** Overrides StartAttack to perform a melee attack. */
	virtual void StartAttack() override;

	//~==============================================================================================
	//~ Properties - Configurable values for melee attack
	//~==============================================================================================

	/** Animation Montage to play when performing a melee attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	TObjectPtr<UAnimMontage> MeleeAttackMontage;

	/** Base damage dealt by this enemy's melee attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	float MeleeDamage = 20.f; // Default damage for melee enemies

	/** Range of the melee attack hitbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	float MeleeAttackRange = 100.f;

	/** Radius of the melee attack hitbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	float MeleeAttackRadius = 50.f;

private:
	/** Controls whether the enemy can attack to prevent spamming. */
	bool bCanAttack = true;
	FTimerHandle TimerHandle_AttackCooldown;

	/** Resets the attack cooldown for the enemy. */
	void ResetAttackCooldown();
};
