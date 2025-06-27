// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	// The weapon is safe, overlaps cause no damage.
	Passive,
	// The weapon is "hot", overlaps cause damage.
	Aggressive 
};

/**
 * AWeapon is a special type of AItem that can cause damage.
 * It has states to control when it actively deals damage.
 */
UCLASS()
class CHAOSRIFTS_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	/**
	 * Sets the state of the weapon.
	 * @param NewState The new state (Passive or Aggressive).
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|State")
	void SetWeaponState(EWeaponState NewState);

	/** Returns the current state of the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|State")
	EWeaponState GetWeaponState() const { return CurrentWeaponState; }

protected:
	virtual void BeginPlay() override;

	// The damage this weapon causes per hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat")
	float Damage;

	// If true, the weapon will not damage its owner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat")
	bool bIgnoreOwner = true;
	
	// A list of specific actor classes to ignore during collision checks.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat")
    TArray<TSubclassOf<AActor>> IgnoredActorClasses;

private:
	// The current state of the weapon. Passive by default.
	UPROPERTY(VisibleAnywhere, Category = "Weapon|State")
	EWeaponState CurrentWeaponState;

	// Function that is called when the mesh component begins to overlap with another actor.
	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// A list of actors that have already received damage in this "attack swing"
	// to prevent them from being hit multiple times per attack.
	UPROPERTY()
	TArray<TObjectPtr<AActor>> DamagedActorsInSwing;
};
