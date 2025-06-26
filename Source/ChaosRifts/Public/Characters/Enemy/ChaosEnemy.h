// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/ChaosCharacterBase.h"
#include "Components/WidgetComponent.h" // For displaying UI widgets above the enemy
#include "ChaosEnemy.generated.h"

/**
 * Base class for all AI-controlled enemies in ChaosRifts.
 * Inherits core attributes and combat logic from AChaosCharacterBase.
 */
UCLASS(abstract)
class CHAOSRIFTS_API AChaosEnemy : public AChaosCharacterBase
{
	GENERATED_BODY()
	
public:
	AChaosEnemy();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

protected:
	//~==============================================================================================
	//~ Combat - Overrides for base combat behavior
	//~==============================================================================================

	/** Overrides the Die_Implementation from AChaosCharacterBase to handle enemy specific death logic. */
	virtual void Die_Implementation() override;

	//~==============================================================================================
	//~ UI - Components for displaying health bars etc.
	//~==============================================================================================

	/** Widget component for displaying the enemy's health bar above their head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	/**
	 * Sets the visibility of the health bar widget.
	 * Can be used to hide it when the enemy is full health or far away.
	 * @param bVisible Whether the health bar should be visible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|UI")
	void SetHealthBarVisibility(bool bVisible);
};
