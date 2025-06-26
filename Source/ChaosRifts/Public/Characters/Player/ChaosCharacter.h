// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/ChaosCharacterBase.h"
#include "Logging/LogMacros.h"
#include "ChaosCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

// Forward Declaration for AChaosEnemy to avoid circular dependencies if needed later, 
// though the include in .cpp will handle most cases.
class AChaosEnemy; 

DECLARE_LOG_CATEGORY_EXTERN(LogChaosCharacter, Log, All);

UENUM(BlueprintType)
enum class EMantleState : uint8
{
	None,
	Reaching,
	PushingForward
};

/**
 * The player-controlled character. Inherits shared functionality from AChaosCharacterBase
 * and adds player-specific logic like input and camera control.
 */
UCLASS(abstract)
class AChaosCharacter : public AChaosCharacterBase
{
	GENERATED_BODY()

public:
	AChaosCharacter();

	//~ Animation Interface
	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	bool IsFalling() const;

	UFUNCTION(BlueprintCallable, Category = "Chaos|Animation")
	bool IsVaulting() const { return bIsVaulting; }
    
	//~ End Animation Interface

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface
	
	//~ Begin APawn Interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface

	// Overriding the Die_Implementation from AChaosCharacterBase for player-specific death logic.
	// We mark it as BlueprintNativeEvent because the base class does, and it's good practice
	// to keep the override consistent, even if its main use here is to trigger Game Over.
	virtual void Die_Implementation() override;

private: // Changed to private for strict encapsulation, but properties are UPROPERTY so accessible in Blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
protected: // Changed to protected for easier access in Blueprint subclasses if any, still meta=AllowPrivateAccess for strictness
	// --- Input Actions ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> DashAction;

	// Input Action for the melee attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> AttackAction;

	// Added: Input Action for spell casting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chaos|Input")
	TObjectPtr<UInputAction> CastSpellAction;

	// --- Input Handlers ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartDash();
	
	// Handler for the melee attack
	void AttackMelee();

	// Added: Handler for spell casting
	void StartSpellCast();

public: // Changed to public for Blueprint Callable functions
	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoLook(float Yaw, float Pitch);

protected: // Changed to protected for easier access in Blueprint subclasses
	// --- Core Movement Properties ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Movement")
	float MovementSpeedDefault = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Movement")
	float JumpVelocityDefault = 800.f;

	// --- Dash Properties ---
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float DashImpulse = 4000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float DashCooldown = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float PostDashSpeedBoostMultiplier = 1.3f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Dash")
	float PostDashSpeedBoostDuration = 1.0f;

	// --- Mantling Properties ---
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleTraceDistance = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MinMantleHeight = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MaxMantleHeight = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleLerpSpeedNormal = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleLerpSpeedFast = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
	float MantleCooldownDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle", meta = (ClampMin = "0.7", ClampMax = "1.0"))
	float MantleActivationDotProduct = 0.8f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Mantle")
    float MantleFastSpeedThreshold = 600.f;

    // --- Animation Montages ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Animation") // Changed to BlueprintReadOnly
    TObjectPtr<UAnimMontage> DashMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Animation") // Changed to BlueprintReadOnly
    TObjectPtr<UAnimMontage> MantleMontage_Normal;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Animation") // Changed to BlueprintReadOnly
    TObjectPtr<UAnimMontage> MantleMontage_Fast;

	// Animation Montages for the melee attack combo
	// Changed from single UAnimMontage to TArray<TObjectPtr<UAnimMontage>>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Animation")
	TArray<TObjectPtr<UAnimMontage>> MeleeAttackMontages;

	// Added: Animation Montage for spell casting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Animation")
	TObjectPtr<UAnimMontage> SpellCastMontage;

	// Damage for the melee attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat") // Changed to BlueprintReadOnly
	float MeleeDamage = 30.f;

	// Added: Chaos cost for spell casting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	float SpellChaosCost = 25.f;

	// Added: Projectile class to spawn for spells
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	TSubclassOf<AActor> SpellProjectileClass;

	// Maximum time after an attack completes to register a combo input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos|Combat")
	float ComboWindowDuration = 0.5f;
	
private:
	// --- Dash System ---
	void ResetDashCooldown();
	void ApplyPostDashSpeedBoost();
	void ResetMovementSpeed();
	
	bool bCanDash = true;
	FTimerHandle TimerHandle_DashCooldown;
	FTimerHandle TimerHandle_PostDashSpeed;

	// --- Mantle System ---
	void TickVaultCheck(float DeltaTime);
	void PerformMantle(const FVector& LandingTarget, const FVector& LedgePosition);
	void TickMantle(float DeltaTime);
	void EndMantle();
	void ResetVaultCooldown();
	
	bool bIsVaulting = false;
	bool bCanCheckVault = true;
	float MantleLerpSpeed = MantleLerpSpeedNormal;
	FVector MantleTargetLocation;
	FVector MantleLedgeLocation;
	EMantleState CurrentMantleState = EMantleState::None;
    TObjectPtr<UAnimMontage> CurrentMantleMontage;
	
	float ForwardInputValue = 0.f;
	float DefaultGravityScale = 1.f;
	FTimerHandle TimerHandle_VaultCooldown;

	// Controls whether the character can attack
	// This will now specifically mean if any attack is in progress or on a global cooldown.
	bool bCanAttack = true; 
	FTimerHandle TimerHandle_GlobalAttackCooldown; // Renamed for clarity

	// --- Melee Combo System ---
	bool bInComboWindow = false; // True if the character just finished an attack and can start a combo
	int32 CurrentComboIndex = 0; // The index of the next montage to play in the combo sequence
	FTimerHandle TimerHandle_ComboWindow; // Timer for the combo input window

	// Resets the combo state
	void ResetCombo();
	// Called when an attack montage finishes playing, to open the combo window
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ResetAttackCooldown(); // This will now reset bCanAttack
	
	// Added: Controls whether the character can cast a spell
	bool bCanCastSpell = true;
	FTimerHandle TimerHandle_SpellCastCooldown;
	void ResetSpellCastCooldown(); // Added: Function to reset spell cast cooldown

	// Added: A specific handler for player death, matching the OnDeath delegate signature.
	UFUNCTION() // UFUNCTION is required for delegates to bind successfully
	void HandlePlayerDeath(AChaosCharacterBase* DeadCharacter);

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
