// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosCharacterBase.generated.h"

class UChaosAttributes;
class AWeapon; // Forward declaration for the weapon class

// A delegate that is broadcast when a character dies.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AChaosCharacterBase*, DeadCharacter);

/**
 * The base class for all characters, now with an extensible weapon system.
 */
UCLASS(abstract)
class CHAOSRIFTS_API AChaosCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacterBase();

	UFUNCTION(BlueprintCallable, Category = "Chaos|Character")
	FORCEINLINE UChaosAttributes* GetAttributes() const { return AttributesComponent; }

	//~==============================================================================================
	//~ Combat Interface
	//~==============================================================================================

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Chaos|Combat")
	void Die();
	virtual void Die_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat")
	virtual void StartAttack();

	UPROPERTY(BlueprintAssignable, Category = "Chaos|Combat")
	FOnDeathDelegate OnDeath;
	
	/** Returns the currently equipped weapon. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	AWeapon* GetCurrentWeapon() const;
	
	//~==============================================================================================
	//~ Weapon System
	//~==============================================================================================
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Character")
	TObjectPtr<UChaosAttributes> AttributesComponent;

	/** The weapon classes that this character should receive by default when spawning. */
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Combat|Weapons")
	TArray<TSubclassOf<AWeapon>> DefaultWeapons;

	/** The array containing the instances of the spawned weapons. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TArray<TObjectPtr<AWeapon>> Weapons;

	/** A pointer to the currently equipped weapon. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TObjectPtr<AWeapon> CurrentWeapon;
	
	/** The index of the currently equipped weapon in the 'Weapons' array. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	int32 CurrentWeaponIndex;

	/**
	 * Spawns the weapons from the DefaultWeapons array and attaches them to the character.
	 * Must be called in derived classes (usually in BeginPlay).
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void SpawnAndEquipWeapons();

	/**
	 * Equips a weapon from the 'Weapons' array.
	 * @param WeaponIndex The index of the weapon to be equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void EquipWeapon(int32 WeaponIndex);

	/** Switches to the next available weapon in the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	void SwapToNextWeapon();

	/** Switches to the previous available weapon in the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	void SwapToPreviousWeapon();
};
