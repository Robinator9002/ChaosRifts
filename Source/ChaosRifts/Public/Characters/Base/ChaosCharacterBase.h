// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Items/Weapons/Weapon.h"
#include "ChaosCharacterBase.generated.h"

class UChaosAttributes;

// A delegate that is broadcast when a character dies.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AChaosCharacterBase*, DeadCharacter);

/**
 * NEW: A struct to define a weapon and its attachment sockets.
 * This will be configured in Blueprints to define a character's loadout.
 */
USTRUCT(BlueprintType)
struct FWeaponLoadoutInfo
{
    GENERATED_BODY()

    // The Blueprint of the weapon to spawn.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Loadout")
    TSubclassOf<AWeapon> WeaponClass;

    // The name of the socket/component for the weapon when HOLSTERED or SHEATHED.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Loadout")
    FName SheathedSocketName;

    // The name of the socket/component for the weapon when ACTIVELY EQUIPPED.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Loadout")
    FName EquippedSocketName;
};


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
	
protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Character")
	TObjectPtr<UChaosAttributes> AttributesComponent;

	//~==============================================================================================
	//~ NEW WEAPON SYSTEM PROPERTIES
	//~==============================================================================================

	/** The character's default weapon loadout, configured in Blueprints. */
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Combat|Weapons")
	TArray<FWeaponLoadoutInfo> DefaultWeaponLoadout;

	/** The array containing the RUNTIME INSTANCES of the spawned weapons. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TArray<TObjectPtr<AWeapon>> Weapons;

	/** A pointer to the currently equipped weapon instance. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TObjectPtr<AWeapon> CurrentWeapon;
	
	/** The index of the currently equipped weapon in the 'Weapons' and 'DefaultWeaponLoadout' arrays. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	int32 CurrentWeaponIndex;
	
	//~==============================================================================================
	//~ NEW WEAPON SYSTEM FUNCTIONS
	//~==============================================================================================
	
	/**
	 * Spawns the weapons from the DefaultWeaponLoadout array and attaches them to their sheathed sockets.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void SpawnAndEquipWeapons();

	/**
	 * Equips a weapon from the 'Weapons' array, moving it from its sheathed socket to its equipped socket.
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

private:
	/**
	 * Helper function to attach a weapon to a specified socket.
	 * It intelligently searches for a USceneComponent first, then falls back to a skeletal mesh socket.
	 * @param WeaponToAttach The weapon actor to attach.
	 * @param SocketName The name of the SceneComponent or skeletal socket.
	 */
	void AttachWeaponToSocket(AWeapon* WeaponToAttach, const FName& SocketName);
};
