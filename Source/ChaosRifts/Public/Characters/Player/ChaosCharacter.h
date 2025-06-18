// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ChaosCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogChaosCharacter, Log, All);

/**
 * Base class for the player character.
 */
UCLASS(abstract)
class AChaosCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacter();

protected:
	//~ Begin AActor Interface
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface
	
	//~ Begin APawn Interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface

private:
	/** Camera boom, positions camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera attached to the boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
protected:
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

	// --- Input Handlers ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartDash();

public:
	// --- Blueprint Callable Input Functions ---
	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Chaos|Input")
	virtual void DoLook(float Yaw, float Pitch);

protected:
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

	// --- Vaulting Properties ---
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Vault")
	float VaultTraceDistance = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Vault")
	float MinVaultHeight = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Vault")
	float MaxVaultHeight = 150.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chaos|Movement|Vault")
	float VaultLerpSpeed = 15.f;
	
private:
	// --- Dash System ---
	void ResetDashCooldown();
	void ApplyPostDashSpeedBoost();
	void ResetMovementSpeed();
	
	bool bCanDash = true;
	FTimerHandle TimerHandle_DashCooldown;
	FTimerHandle TimerHandle_PostDashSpeed;

	// --- Vault System ---
	void TickVaultCheck(float DeltaTime);
	void PerformVault(const FVector& VaultStart, const FVector& VaultEnd);
	
	bool bIsVaulting = false;
	FVector VaultTargetLocation;
	float ForwardInputValue = 0.f; // Cached input value for vault check

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
