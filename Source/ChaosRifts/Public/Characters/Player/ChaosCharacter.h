// Copyright Robinator Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/ChaosCharacterBase.h" // MODIFIED: Inherit from our new base class
#include "Logging/LogMacros.h"
#include "ChaosCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

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
class AChaosCharacter : public AChaosCharacterBase // MODIFIED: Changed inheritance from ACharacter
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

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

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
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> DashMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> MantleMontage_Normal;
    
    UPROPERTY(EditDefaultsOnly, Category = "Chaos|Animation")
    TObjectPtr<UAnimMontage> MantleMontage_Fast;
	
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

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
