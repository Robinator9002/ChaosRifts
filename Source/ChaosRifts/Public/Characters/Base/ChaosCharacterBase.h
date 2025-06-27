// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosCharacterBase.generated.h"

class UChaosAttributes;
class AWeapon; // Forward declaration für die Waffenklasse

// A delegate that is broadcast when a character dies.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AChaosCharacterBase*, DeadCharacter);

/**
 * Die Basisklasse für alle Charaktere, jetzt mit einem erweiterbaren Waffensystem.
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
	
	/** Gibt die aktuell ausgerüstete Waffe zurück. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat")
	AWeapon* GetCurrentWeapon() const;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChaosAttributes> AttributesComponent;

	//~==============================================================================================
	//~ Weapon System
	//~==============================================================================================

	/** Eine Liste von Waffenklassen, die dieser Charakter bei Spielbeginn erhalten soll. */
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Combat|Weapons")
	TArray<TSubclassOf<AWeapon>> DefaultWeapons;

	/** Das Array, das die Instanzen der gespawnten Waffen enthält. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TArray<TObjectPtr<AWeapon>> Weapons;

	/** Ein Zeiger auf die aktuell ausgerüstete Waffe. */
	UPROPERTY(BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	TObjectPtr<AWeapon> CurrentWeapon;
	
	/** Der Index der aktuell ausgerüsteten Waffe im 'Weapons'-Array. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Chaos|Combat|Weapons")
	int32 CurrentWeaponIndex;

	/**
	 * Spawnt die Waffen aus dem DefaultWeapons-Array und hängt sie an den Charakter.
	 * Muss in abgeleiteten Klassen aufgerufen werden (normalerweise in BeginPlay).
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void SpawnAndEquipWeapons();

	/**
	 * Rüstet eine Waffe aus dem 'Weapons'-Array aus.
	 * @param WeaponIndex Der Index der Waffe, die ausgerüstet werden soll.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void EquipWeapon(int32 WeaponIndex);

	/** Wechselt zur nächsten verfügbaren Waffe im Inventar. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void SwapToNextWeapon();
	
	/** Wechselt zur vorherigen verfügbaren Waffe im Inventar. */
	UFUNCTION(BlueprintCallable, Category = "Chaos|Combat|Weapons")
	virtual void SwapToPreviousWeapon();
};
