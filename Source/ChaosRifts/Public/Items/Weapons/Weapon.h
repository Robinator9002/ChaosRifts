// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	// Die Waffe ist sicher, Overlaps verursachen keinen Schaden.
	Passive,
	// Die Waffe ist "heiß", Overlaps verursachen Schaden.
	Aggressive 
};

/**
 * AWeapon ist eine spezielle Art von AItem, die Schaden verursachen kann.
 * Sie hat Zustände, um zu kontrollieren, wann sie aktiv Schaden austeilt.
 */
UCLASS()
class CHAOSRIFTS_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	// Das Mesh der Waffe
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	/**
	 * Setzt den Zustand der Waffe.
	 * @param NewState Der neue Zustand (Passiv oder Aggressiv).
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|State")
	void SetWeaponState(EWeaponState NewState);

	/** Gibt den aktuellen Zustand der Waffe zurück. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|State")
	EWeaponState GetWeaponState() const { return CurrentWeaponState; }

protected:
	virtual void BeginPlay() override;

	// Der Schaden, den diese Waffe pro Treffer verursacht.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combat")
	float Damage;

private:
	// Der aktuelle Zustand der Waffe. Standardmäßig passiv.
	UPROPERTY(VisibleAnywhere, Category = "Weapon|State")
	EWeaponState CurrentWeaponState;

	// Funktion, die aufgerufen wird, wenn das OnItemOverlap-Delegate der Basisklasse feuert.
	UFUNCTION()
	void HandleItemOverlap(AActor* OverlappedActor, bool bIsOverlapping);

	// Eine Liste von Actors, die in diesem "Angriffsschwung" bereits Schaden erhalten haben,
	// um zu verhindern, dass ein einzelner Schwung mehrfach Schaden verursacht.
	TArray<TObjectPtr<AActor>> DamagedActorsInSwing;
};
